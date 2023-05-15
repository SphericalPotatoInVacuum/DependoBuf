#include "core/checker/type_checker.h"

#include "core/ast/ast.h"
#include "core/ast/expression.h"
#include "core/checker/common.h"
#include "core/interning/interned_string.h"
#include "glog/logging.h"
#include "location.hh"
#include "z3++.h"

#include <cstddef>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

namespace dbuf::checker {

TypeChecker::TypeChecker(const ast::AST &ast, const std::vector<InternedString> &sorted_graph)
    : ast_(ast)
    , sorted_graph_(sorted_graph) {}

ErrorList TypeChecker::CheckTypes() {
  BuildConstructorToEnum();

  for (const auto &node : sorted_graph_) {
    std::visit(*this, ast_.types.at(node));
  }

  return errors_;
}

void TypeChecker::operator()(const ast::Message &ast_message) {
  DLOG(INFO) << "Checking message: " << ast_message.identifier.name;

  // Scope of the message checked
  auto scope = Scope(&context_);

  // First step is to check if dependencies' types are correctly constructed
  CheckDependencies(ast_message);

  // Same with fields
  CheckFields(ast_message);

  // Create a z3 datatype for this message

  z3::symbol name_symbol = z3_stuff_.context_.str_symbol(ast_message.identifier.name.GetString().c_str());

  z3::constructors cs(z3_stuff_.context_);
  z3_stuff_.sorts_.emplace(ast_message.identifier.name, z3_stuff_.context_.datatype_sort(name_symbol));

  z3::symbol recognizer_symbol =
      z3_stuff_.context_.str_symbol(("is_" + ast_message.identifier.name.GetString()).c_str());
  std::vector<z3::symbol> accessor_names;
  std::vector<z3::sort> accessor_sorts;

  for (const auto &field : ast_message.fields) {
    accessor_names.push_back(z3_stuff_.context_.str_symbol(field.name.GetString().c_str()));
    accessor_sorts.push_back(z3_stuff_.sorts_.at(field.type_expression.identifier.name));
  }

  cs.add(name_symbol, recognizer_symbol, ast_message.fields.size(), accessor_names.data(), accessor_sorts.data());

  z3_stuff_.sorts_.at(ast_message.identifier.name) = z3_stuff_.context_.datatype(name_symbol, cs);
  DLOG(INFO) << "Created a message sort: " << z3_stuff_.sorts_.at(ast_message.identifier.name);

  // Declare a constructor and accessors for this message
  z3::func_decl message_constructor(z3_stuff_.context_);
  z3::func_decl is_message_constructor_recognizer(z3_stuff_.context_);
  z3::func_decl_vector field_accessors(z3_stuff_.context_);

  // Query them from the constructors
  cs.query(0, message_constructor, is_message_constructor_recognizer, field_accessors);

  z3_stuff_.constructors_.emplace(ast_message.identifier.name, message_constructor);
  DLOG(INFO) << "Constructor: " << message_constructor;

  z3_stuff_.accessors_.emplace(ast_message.identifier.name, Z3stuff::FieldToAccessor());
  for (size_t i = 0; i < ast_message.fields.size(); ++i) {
    z3_stuff_.accessors_.at(ast_message.identifier.name).emplace(ast_message.fields[i].name, field_accessors[i]);
    DLOG(INFO) << "Accessor for field " << ast_message.fields[i].name << ": " << field_accessors[i];
  }

  DLOG(INFO) << "Finished checking message: " << ast_message.identifier.name;
}

void TypeChecker::operator()(const ast::Enum &ast_enum) {
  DLOG(INFO) << "Checking enum: " << ast_enum.identifier.name;
  // Scope of the enum checked
  auto scope = Scope(&context_);

  // First step is to check if dependencies' types are correctly constructed
  CheckDependencies(ast_enum);

  // The next step is to check the pattern mathcing
  for (const auto &rule : ast_enum.pattern_mapping) {
    DLOG(INFO) << "Checking rule with inputs " << rule.inputs;
    // We need separate scope for each pattern
    auto scope = Scope(&context_);
    substitutor_.PushScope();

    if (rule.inputs.size() != ast_enum.type_dependencies.size()) {
      AddError(&errors_) << "Expected " << ast_enum.type_dependencies.size() << " inputs in pattern for enum \""
                         << ast_enum.identifier.name << "\", but got " << rule.inputs.size();
      return;
    }

    // Check input patterns
    for (size_t id = 0; id < rule.inputs.size(); ++id) {
      // We ignore Stars as they match any type and do not declare new variables
      if (std::holds_alternative<ast::Star>(rule.inputs[id])) {
        continue;
      }

      const auto &value = std::get<ast::Value>(rule.inputs[id]);
      substitutor_.AddSubstitution(ast_enum.type_dependencies[id].name, std::make_shared<const ast::Expression>(value));

      // Check that value has expected type in this context
      TypeComparator(ast::Expression(value), ast_enum.type_dependencies[id].type_expression, this).Compare();

      // Now we need to add newly declared variables to the context

      // Scalar values do not declare new variables, so we skip them
      if (!std::holds_alternative<ast::ConstructedValue>(value)) {
        continue;
      }

      const auto &constructed_value = std::get<ast::ConstructedValue>(value);

      // Find constructor used to construct the value
      InternedString type_name            = ast_enum.type_dependencies[id].type_expression.identifier.name;
      const auto &type                    = ast_.types.at(type_name);
      ast::TypeWithFields const *type_ptr = nullptr;
      ast::Identifier identifier;
      if (std::holds_alternative<ast::Enum>(type)) {
        const auto &target_enum = std::get<ast::Enum>(type);

        // Looking for the constructor
        for (const auto &pattern : target_enum.pattern_mapping) {
          for (const auto &output : pattern.outputs) {
            if (output.identifier.name == constructed_value.constructor_identifier.name) {
              type_ptr   = &output;
              identifier = output.identifier;
              break;
            }
          }
        }
      } else {
        type_ptr   = &std::get<ast::Message>(type);
        identifier = std::get<ast::Message>(type).identifier;
      }

      const auto &type_with_fields = *type_ptr;

      if (type_with_fields.fields.size() != constructed_value.fields.size()) {
        AddError(&errors_) << "Expected " << type_with_fields.fields.size() << " fields in constructor \""
                           << identifier.name << "\", but got " << constructed_value.fields.size() << " at "
                           << constructed_value.location;
        return;
      }

      // Adding new variables to scope
      for (size_t j = 0; j < type_with_fields.fields.size(); ++j) {
        if (!std::holds_alternative<ast::VarAccess>(*constructed_value.fields[j].second)) {
          continue;
        }
        ast::VarAccess var_access = std::get<ast::VarAccess>(*constructed_value.fields[j].second);

        // If new variable
        if (var_access.field_identifiers.empty()) {
          scope.AddName(var_access.var_identifier.name, type_with_fields.fields[j].type_expression);
        }
      }
    }

    for (const auto &constructor : rule.outputs) {
      DLOG(INFO) << "Checking constructor: " << constructor.identifier.name;
      CheckFields(constructor);
    }

    substitutor_.PopScope();
  }

  // Create a z3 sort for this enum

  z3::constructors cs(z3_stuff_.context_);
  z3::symbol name_symbol = z3_stuff_.context_.str_symbol(ast_enum.identifier.name.GetString().c_str());
  z3_stuff_.sorts_.emplace(ast_enum.identifier.name, z3_stuff_.context_.datatype_sort(name_symbol));

  for (const auto &rule : ast_enum.pattern_mapping) {
    for (const auto &constructor : rule.outputs) {
      std::vector<z3::symbol> accessor_names;
      std::vector<z3::sort> accessor_sorts;

      for (const auto &field : constructor.fields) {
        accessor_names.push_back(z3_stuff_.context_.str_symbol(field.name.GetString().c_str()));
        accessor_sorts.push_back(z3_stuff_.sorts_.at(field.type_expression.identifier.name));
      }

      cs.add(
          z3_stuff_.context_.str_symbol(constructor.identifier.name.GetString().c_str()),
          z3_stuff_.context_.str_symbol(("is_" + constructor.identifier.name.GetString()).c_str()),
          accessor_names.size(),
          accessor_names.data(),
          accessor_sorts.data());
    }
  }

  z3_stuff_.sorts_.at(ast_enum.identifier.name) = z3_stuff_.context_.datatype(name_symbol, cs);
  DLOG(INFO) << "Created enum datatype: " << z3_stuff_.sorts_.at(ast_enum.identifier.name);

  size_t constructor_idx = 0;
  for (const auto &rule : ast_enum.pattern_mapping) {
    for (const auto &constructor : rule.outputs) {
      z3::func_decl z3_constructor(z3_stuff_.context_);
      z3::func_decl is_z3_constructor_recognizer(z3_stuff_.context_);
      z3::func_decl_vector field_accessors(z3_stuff_.context_);

      cs.query(constructor_idx, z3_constructor, is_z3_constructor_recognizer, field_accessors);

      z3_stuff_.constructors_.emplace(constructor.identifier.name, z3_constructor);
      DLOG(INFO) << "Constructor \"" << constructor.identifier.name << "\": " << z3_constructor;

      z3_stuff_.accessors_.emplace(constructor.identifier.name, Z3stuff::FieldToAccessor());
      for (size_t i = 0; i < constructor.fields.size(); ++i) {
        z3_stuff_.accessors_.at(constructor.identifier.name).emplace(constructor.fields[i].name, field_accessors[i]);
        DLOG(INFO) << "Accessor for field " << constructor.fields[i].name << ": " << field_accessors[i];
      }

      ++constructor_idx;
    }
  }

  DLOG(INFO) << "Finished checking enum: " << ast_enum.identifier.name;
}

void TypeChecker::CheckDependencies(const ast::DependentType &type) {
  Scope &scope = *context_.back();
  DLOG(INFO) << "Checking dependencies";
  for (const auto &dependency : type.type_dependencies) {
    DLOG(INFO) << "Checking dependency: " << dependency;
    CheckTypeExpression(dependency.type_expression);

    // After we checked the depdency we can add it to scope to be seen by other dependencies
    scope.AddName(dependency.name, dependency.type_expression);
  }
  DLOG(INFO) << "Finished checking dependencies";
}

void TypeChecker::CheckFields(const ast::TypeWithFields &type) {
  DLOG(INFO) << "Checking fields";
  Scope scope(&context_);

  for (const auto &field : type.fields) {
    DLOG(INFO) << "Checking field: " << field.name << " of type " << field.type_expression;
    auto after_substitution = std::get<ast::TypeExpression>(substitutor_(field.type_expression));
    DLOG(INFO) << "After substitution: " << after_substitution;
    CheckTypeExpression(after_substitution);
    scope.AddName(field.name, after_substitution);
  }
}

void TypeChecker::CheckTypeExpression(const ast::TypeExpression &type_expression) {
  DLOG(INFO) << "Checking type expression: " << type_expression;
  if (type_expression.identifier.name == InternedString("Int") ||
      type_expression.identifier.name == InternedString("Unsigned") ||
      type_expression.identifier.name == InternedString("Bool") ||
      type_expression.identifier.name == InternedString("String")) {
    return;
  }

  const auto &type_variant = ast_.types.at(type_expression.identifier.name);
  const ast::DependentType &type =
      std::visit([](const auto &type) { return static_cast<ast::DependentType>(type); }, type_variant);

  // Number of parameters should be equal to number of dependencies of message/enum
  if (type.type_dependencies.size() != type_expression.parameters.size()) {
    AddError(&errors_) << "Expected " << type.type_dependencies.size() << " parameters for typename \""
                       << type_expression.identifier.name << "\", but got " << type_expression.parameters.size()
                       << " at " << type_expression.location;
    return;
  }

  DLOG(INFO) << "Checking parameters";

  // Next step is to check parameters
  substitutor_.PushScope();
  for (size_t id = 0; id < type.type_dependencies.size(); ++id) {
    DLOG(INFO) << "Comparing argument " << *type_expression.parameters[id] << " against type "
               << type.type_dependencies[id].type_expression;
    // Update type using already known substitutions
    ast::TypeExpression substituted_type =
        std::get<ast::TypeExpression>(substitutor_(type.type_dependencies[id].type_expression));
    DLOG(INFO) << "Type after substitution: " << substituted_type;

    // Check that parameter hase exprected type
    TypeComparator(*type_expression.parameters[id], substituted_type, this).Compare();

    // Now we can update substitutions
    substitutor_.AddSubstitution(type.type_dependencies[id].name, type_expression.parameters[id]);
  }
  substitutor_.PopScope();
}

bool TypeChecker::CompareExpressions(const ast::Expression &expected, const ast::Expression &got) {
  DLOG(INFO) << "Comparing expressions " << expected << " and " << got;
  z3_stuff_.solver_.push();

  ExpressionToZ3 converter {z3_stuff_};
  z3::expr expected_z3_expr = converter(expected);
  DLOG(INFO) << "Expected z3 expr: " << expected_z3_expr;
  z3::expr got_z3_expr = converter(got);
  DLOG(INFO) << "Got z3 expr: " << got_z3_expr;

  z3_stuff_.solver_.add(expected_z3_expr != got_z3_expr);

  bool result = z3_stuff_.solver_.check() == z3::unsat;

  z3_stuff_.solver_.pop();

  DLOG(INFO) << "Equal? " << result;

  return result;
}

bool TypeChecker::CompareTypeExpressions(
    const ast::TypeExpression &expected_type,
    const ast::TypeExpression &expression) {
  if (expected_type.identifier.name != expression.identifier.name) {
    AddError(&errors_) << "Got type \"" << expression.identifier.name << "\", but expected type is \""
                       << expected_type.identifier.name << "\" at " << expression.location;
    return false;
  }

  if (expected_type.parameters.size() != expression.parameters.size()) {
    AddError(&errors_) << "Expected " << expected_type.parameters.size() << "type parametes, but got "
                       << expression.parameters.size() << " at " << expression.location;
    return false;
  }

  for (size_t id = 0; id < expected_type.parameters.size(); ++id) {
    if (!CompareExpressions(*expected_type.parameters[id], *expression.parameters[id])) {
      return false;
    }
  }

  return true;
}

void TypeChecker::BuildConstructorToEnum() {
  for (const auto &[type_name, type_variant] : ast_.types) {
    if (std::holds_alternative<ast::Enum>(type_variant)) {
      const auto &ast_enum = std::get<ast::Enum>(type_variant);
      for (const auto &rule : ast_enum.pattern_mapping) {
        for (const auto &constructor : rule.outputs) {
          constructor_to_enum_.emplace(constructor.identifier.name, ast_enum.identifier.name);
        }
      }
    } else {
      const auto &ast_message = std::get<ast::Message>(type_variant);
      constructor_to_enum_.emplace(ast_message.identifier.name, ast_message.identifier.name);
    }
  }
}

void TypeChecker::Scope::AddName(InternedString name, ast::TypeExpression type) {
  vars_.insert_or_assign(name, std::move(type));
  DLOG(INFO) << "Added name \"" << name << "\" with type \"" << vars_.at(name) << "\" to context";
}

[[nodiscard]] const ast::TypeExpression &TypeChecker::Scope::LookupName(InternedString name) const {
  for (const auto &scope : std::ranges::reverse_view(ctx_)) {
    auto it = scope->vars_.find(name);
    if (it != scope->vars_.end()) {
      return it->second;
    }
  }
  throw std::runtime_error("Can't find name \"" + name.GetString() + "\"");
}

void TypeChecker::TypeComparator::Compare() {
  auto result = std::visit(*this, expr_);
  if (result) {
    DLOG(INFO) << "Type of expression " << expr_ << " is " << *result;
  }
}

std::optional<ast::TypeExpression> TypeChecker::TypeComparator::operator()(const ast::TypeExpression &) {
  throw std::logic_error("TypeExpression should not be compared");
}
std::optional<ast::TypeExpression> TypeChecker::TypeComparator::operator()(const ast::BinaryExpression &expr) {
  DLOG(INFO) << "Checking binary expression " << expr;
  auto left_type  = std::visit(*this, *expr.left);
  auto right_type = std::visit(*this, *expr.right);
  if (!left_type || !right_type) {
    DLOG(ERROR) << "Type mismatch in expression " << expr;
    return {};
  }
  if (!checker_.CompareTypeExpressions(*left_type, *right_type)) {
    DLOG(ERROR) << "Type mismatch in expression " << expr;
    return {};
  }
  DLOG(INFO) << "Expression " << expr << " should be of type " << *left_type;
  if (expr.type == ast::BinaryExpressionType::Plus) {
    if (left_type->identifier.name == InternedString("Int") ||
        left_type->identifier.name == InternedString("Unsigned") ||
        left_type->identifier.name == InternedString("String") ||
        left_type->identifier.name == InternedString("Float")) {
      return left_type;
    }
  }
  if (expr.type == ast::BinaryExpressionType::Star) {
    if (left_type->identifier.name == InternedString("Int") ||
        left_type->identifier.name == InternedString("Unsigned") ||
        left_type->identifier.name == InternedString("Float")) {
      return left_type;
    }
  }
  if (expr.type == ast::BinaryExpressionType::Minus) {
    if (left_type->identifier.name == InternedString("Int") || left_type->identifier.name == InternedString("Float")) {
      return left_type;
    }
  }
  if (expr.type == ast::BinaryExpressionType::Slash) {
    if (left_type->identifier.name == InternedString("Float")) {
      return left_type;
    }
  }
  if (expr.type == ast::BinaryExpressionType::And || expr.type == ast::BinaryExpressionType::Or) {
    if (left_type->identifier.name == InternedString("Bool")) {
      return left_type;
    }
  }
  DLOG(ERROR) << "Invalid operator use in expression " << expr;
  AddError(&checker_.errors_) << "Operator \"" << static_cast<char>(expr.type) << "\""
                              << " is not supported by type " << left_type->identifier.name << "\" at "
                              << expr.location;
  return {};
}
std::optional<ast::TypeExpression> TypeChecker::TypeComparator::operator()(const ast::UnaryExpression &expr) {
  DLOG(INFO) << "Checking unary expression " << expr;
  auto type = std::visit(*this, *expr.expression);
  if (expr.type == ast::UnaryExpressionType::Minus) {
    if (type->identifier.name == InternedString("Int") || type->identifier.name == InternedString("Float")) {
      return type;
    }
  }
  if (expr.type == ast::UnaryExpressionType::Bang) {
    if (type->identifier.name == InternedString("Bool")) {
      return type;
    }
  }
  DLOG(INFO) << "Invalid operator use in expression " << expr;
  AddError(&checker_.errors_) << "Operator \"" << static_cast<char>(expr.type) << "\""
                              << " is not supported by type " << type->identifier.name << "\" at " << expr.location;
  return {};
}
std::optional<ast::TypeExpression> TypeChecker::TypeComparator::operator()(const ast::VarAccess &expr) {
  const Scope &outer_scope = *checker_.context_.back();
  DLOG(INFO) << "Checking var access: " << expr;
  // Case: does foo has type Foo?
  if (expr.field_identifiers.empty()) {
    // We can find foo in scope and just compare its type
    if (checker_.CompareTypeExpressions(expected_, outer_scope.LookupName(expr.var_identifier.name))) {
      return {expected_};
    }
    return {};
  }
  // Else we have case: does foo.bar.buzz has type Buzz?

  // First lets find typename of foo. We have foo in scope, so we can get it from there
  InternedString message_name = outer_scope.LookupName(expr.var_identifier.name).identifier.name;
  // The field we are looking for. In case foo.bar.buzz we are looking for bar
  InternedString expected_field = expr.field_identifiers[0].name;

  const auto &type = checker_.ast_.types.at(message_name);
  if (std::holds_alternative<ast::Enum>(type)) {
    AddError(&checker_.errors_) << "Field access works only for messages, but \"" << message_name << "\" is enum";
    return {};
  }
  const auto &message = std::get<ast::Message>(type);

  bool found = false;

  // Let's notice the Type(foo.bar.buzz) == Type(bar.buzz) == Type (buzz)
  // We want to check type recursively, so I add new scope -- the scope of message Type(foo)
  Scope scope(&checker_.context_);
  for (const auto &dependencie : message.type_dependencies) {
    scope.AddName(dependencie.name, dependencie.type_expression);
  }
  for (const auto &field : message.fields) {
    scope.AddName(field.name, field.type_expression);
    if (field.name == expected_field) {
      found = true;
      break;
    }
  }

  // This case is not checked in name resolution checker, so I do it there
  if (!found) {
    AddError(&checker_.errors_) << "Field \"" << expected_field << "\" not found in message \"" << message_name << "\"";
    return {};
  }

  // Recursion
  ast::VarAccess var_access;
  var_access.var_identifier    = {{parser::location()}, {expected_field}};
  var_access.field_identifiers = std::vector<ast::Identifier>();
  for (size_t id = 1; id < expr.field_identifiers.size(); ++id) {
    var_access.field_identifiers.push_back(expr.field_identifiers[id]);
  }

  (*this)(var_access);

  return {expected_};
};

std::optional<ast::TypeExpression> TypeChecker::TypeComparator::operator()(const ast::Value &val) {
  return std::visit(*this, val);
}

std::optional<ast::TypeExpression> TypeChecker::TypeComparator::operator()(const ast::ConstructedValue &val) {
  const InternedString &enum_identifier = checker_.constructor_to_enum_.at(val.constructor_identifier.name);
  // TODO(bug): Constructed value can also be a message, not only enum

  if (enum_identifier != expected_.identifier.name) {
    AddError(&checker_.errors_) << "Got value of type \"" << enum_identifier << "\", but expected type is \""
                                << expected_.identifier.name << "\" at " << val.location;
    return {};
  }

  // TODO(alisa-vernigor): The pattern matching problem must be solved to check that the
  // constructor used creates the required type.
  return expected_;
}

} // namespace dbuf::checker
