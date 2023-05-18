#include "core/checker/type_comparator.h"

#include "core/ast/ast.h"
#include "core/ast/expression.h"
#include "core/checker/common.h"
#include "core/checker/expression_comparator.h"
#include "core/interning/interned_string.h"
#include "core/substitutor/substitutor.h"
#include "glog/logging.h"

#include <optional>
#include <variant>

namespace dbuf::checker {

struct Matcher {
  Z3stuff &z3_stuff;
  std::deque<Scope *> &context;
  Substitutor &substitutor;
  const ast::AST &ast;

  bool operator()(const ast::Expression &arg, const ast::Enum::Rule::InputPattern &pattern) {
    bool matches = std::visit(*this, arg, pattern);
    DLOG(INFO) << arg << (matches ? " matches " : " does not match ") << pattern;
    return matches;
  }

  // Anything matches the star
  template <typename T>
  bool operator()(const T &arg, const ast::Star & /*star*/) {
    DLOG(INFO) << "Matched " << arg << " to Star";
    return true;
  }

  // Anything matches the var access
  template <typename T>
  bool operator()(const T &arg, const ast::VarAccess &pattern) {
    DLOG(INFO) << "Matched " << arg << " to VarAccess " << pattern;
    substitutor.AddSubstitution(pattern.var_identifier.name, std::make_shared<ast::Expression>(arg));
    return true;
  }

  bool operator()(const ast::Expression &arg, const ast::Value &pattern) {
    return std::visit(*this, arg, pattern);
  }

  bool operator()(const ast::Value &arg, const ast::Value &pattern) {
    return std::visit(*this, arg, pattern);
  }

  bool operator()(const ast::ConstructedValue &arg, const ast::ConstructedValue &pattern) {
    DLOG(INFO) << "Matching constructed value " << arg << " against " << pattern;
    if (arg.constructor_identifier.name != pattern.constructor_identifier.name) {
      return false;
    }
    if (arg.fields.size() != pattern.fields.size()) {
      return false;
    }
    for (size_t i = 0; i < arg.fields.size(); ++i) {
      const auto &[field_name, pattern_field] = pattern.fields[i];
      const auto &[_, arg_field]              = arg.fields[i];
      DLOG(INFO) << "Matching field " << field_name.name;
      if (!std::visit(*this, *arg_field, *pattern_field)) {
        DLOG(INFO) << *arg_field << " didn't match " << *pattern_field << " in field " << field_name.name;
        return false;
      }
    }
    return true;
  }
  template <typename T>
  bool operator()(const T &arg, const ast::ConstructedValue &pattern) {
    DLOG(INFO) << "Tried to match " << arg << " against ConstructedValue " << pattern;
    return false;
  }
  template <>
  bool operator()(const ast::VarAccess &arg, const ast::ConstructedValue &pattern) {
    const auto &type_variant = ast.types.at(GetVarAccessType(arg, ast, &context).identifier.name);
    if (std::holds_alternative<ast::Enum>(type_variant)) {
      return false;
    }
    for (const auto &[field_name, field] : pattern.fields) {
      ast::VarAccess arg_field = arg;
      arg_field.field_identifiers.push_back(field_name);
      if (!(*this)(arg_field, *field)) {
        return false;
      }
    }
    return true;
  }

  template <typename T>
  bool operator()(const ast::ConstructedValue &arg, const T &pattern) {
    DLOG(INFO) << "Tried to match ConstructedValue " << arg << " against " << pattern;
    return false;
  }

  template <typename T, typename U>
  bool operator()(const T &arg, const U &pattern) {
    // CompareExpressions returns std::optional<Error> if the expressions don't match
    // so if there is no value then the expressions match
    return !CompareExpressions(pattern, arg, z3_stuff, ast, context).has_value();
  }
};

std::optional<Error> TypeComparator::Compare(const ast::Expression &expr) {
  auto result = std::visit(*this, expr);
  if (result) {
    DLOG(ERROR) << result->message;
  } else {
    DLOG(INFO) << "Expression " << expr << " is of expected type " << expected_;
  }
  return result;
}

std::optional<Error> TypeComparator::operator()(const ast::TypeExpression &) {
  LOG(FATAL) << "TypeExpression should not be compared";
}
std::optional<Error> TypeComparator::operator()(const ast::BinaryExpression &expr) {
  DLOG(INFO) << "Checking binary expression " << expr;
  auto left_type_err = std::visit(*this, *expr.left);
  if (left_type_err) {
    return left_type_err;
  }
  auto right_type_err = std::visit(*this, *expr.right);
  if (right_type_err) {
    return right_type_err;
  }

  DLOG(INFO) << "Expression " << expr << " should be of type " << expected_;
  if (expr.type == ast::BinaryExpressionType::Plus) {
    if (expected_.identifier.name == InternedString("Int") || expected_.identifier.name == InternedString("Unsigned") ||
        expected_.identifier.name == InternedString("String") || expected_.identifier.name == InternedString("Float")) {
      return {};
    }
  }
  if (expr.type == ast::BinaryExpressionType::Star) {
    if (expected_.identifier.name == InternedString("Int") || expected_.identifier.name == InternedString("Unsigned") ||
        expected_.identifier.name == InternedString("Float")) {
      return {};
    }
  }
  if (expr.type == ast::BinaryExpressionType::Minus) {
    if (expected_.identifier.name == InternedString("Int") || expected_.identifier.name == InternedString("Float")) {
      return {};
    }
  }
  if (expr.type == ast::BinaryExpressionType::Slash) {
    if (expected_.identifier.name == InternedString("Float")) {
      return {};
    }
  }
  if (expr.type == ast::BinaryExpressionType::And || expr.type == ast::BinaryExpressionType::Or) {
    if (expected_.identifier.name == InternedString("Bool")) {
      return {};
    }
  }
  DLOG(ERROR) << "Invalid operator use in expression " << expr;
  return Error(
      CreateError() << "Operator \"" << static_cast<char>(expr.type) << "\""
                    << " is not supported by type " << expected_.identifier.name << "\" at " << expr.location);
  return {};
}
std::optional<Error> TypeComparator::operator()(const ast::UnaryExpression &expr) {
  DLOG(INFO) << "Checking unary expression " << expr;
  auto expr_err = std::visit(*this, *expr.expression);
  if (expr_err) {
    return expr_err;
  }
  if (expr.type == ast::UnaryExpressionType::Minus) {
    if (expected_.identifier.name == InternedString("Int") || expected_.identifier.name == InternedString("Float")) {
      return {};
    }
  }
  if (expr.type == ast::UnaryExpressionType::Bang) {
    if (expected_.identifier.name == InternedString("Bool")) {
      return {};
    }
  }
  DLOG(ERROR) << "Invalid operator use in expression " << expr;
  return Error(
      CreateError() << "Operator \"" << static_cast<char>(expr.type) << "\""
                    << " is not supported by type " << expected_.identifier.name << "\" at " << expr.location);
  return {};
}
std::optional<Error> TypeComparator::operator()(const ast::VarAccess &expr) {
  const Scope &outer_scope = *context_.back();
  DLOG(INFO) << "Checking var access: " << expr;
  // Case: does foo has type Foo?
  if (expr.field_identifiers.empty()) {
    // We can find foo in scope and just compare its type
    return CompareTypeExpressions(expected_, outer_scope.LookupName(expr.var_identifier.name), z3_stuff_);
  }
  // Else we have case: does foo.bar.buzz has type Buzz?

  // First lets find typename of foo. We have foo in scope, so we can get it from there
  InternedString message_name = outer_scope.LookupName(expr.var_identifier.name).identifier.name;
  // The field we are looking for. In case foo.bar.buzz we are looking for bar
  InternedString expected_field = expr.field_identifiers[0].name;

  const auto &type = ast_.types.at(message_name);
  if (std::holds_alternative<ast::Enum>(type)) {
    return Error(CreateError() << "Field access works only for messages, but \"" << message_name << "\" is enum");
    return {};
  }
  const auto &message = std::get<ast::Message>(type);

  bool found = false;

  // Let's notice the Type(foo.bar.buzz) == Type(bar.buzz) == Type (buzz)
  // We want to check type recursively, so I add new scope -- the scope of message Type(foo)
  Scope scope(&context_);
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
    return Error(CreateError() << "Field \"" << expected_field << "\" not found in message \"" << message_name << "\"");
  }

  // Recursion
  ast::VarAccess var_access;
  var_access.var_identifier    = {{parser::location()}, {expected_field}};
  var_access.field_identifiers = std::vector<ast::Identifier>();
  for (size_t id = 1; id < expr.field_identifiers.size(); ++id) {
    var_access.field_identifiers.push_back(expr.field_identifiers[id]);
  }

  (*this)(var_access);

  return {};
};

std::optional<Error> TypeComparator::operator()(const ast::Value &val) {
  return std::visit(*this, val);
}

std::optional<Error> TypeComparator::operator()(const ast::ConstructedValue &val) {
  const InternedString &type_name = ast_.constructor_to_type.at(val.constructor_identifier.name);

  if (type_name != expected_.identifier.name) {
    DLOG(ERROR) << "Invalid type of constructed value " << val << " expected " << expected_.identifier.name << " got "
                << type_name << " at " << val.location;
    return Error(
        CreateError() << "Got value of type \"" << type_name << "\", but expected type is \""
                      << expected_.identifier.name << "\" at " << val.location);
  }

  // If the base type is correct then we have two cases:
  // 1. The value is a message. No extra work needed
  if (std::holds_alternative<ast::Message>(ast_.types.at(type_name))) {
    DLOG(INFO) << "Constructed value " << val << " has a message type " << type_name << ", checking fields";
    const auto &ast_message = std::get<ast::Message>(ast_.types.at(type_name));
    return CheckConstructedValue(val, ast_message);
  }
  DLOG(INFO) << "Constructed value " << val << " has an enum type " << type_name
             << ", checking constructor availability";
  const auto &ast_enum = std::get<ast::Enum>(ast_.types.at(type_name));
  // 2. The value is an enum. Then we need to check that the constructor can be used after pattern matching
  // the arguments that were passed in our expected type expression
  // To do this we need to iterate through rules and find the one that can be satisfied or throw an error if none can
  DLOG(INFO) << "Matching expression " << expected_ << " against enum " << type_name;
  for (const auto &rule : ast_enum.pattern_mapping) {
    DLOG(INFO) << "Trying input patterns " << rule.inputs;
    bool matches = true;
    substitutor_.PushScope();
    for (size_t i = 0; i < rule.inputs.size(); ++i) {
      if (!Matcher {z3_stuff_, context_, substitutor_, ast_}(*expected_.parameters[i], rule.inputs[i])) {
        matches = false;
        break;
      }
      // If the pattern matches, we need to add the bindings to the substitutor
      substitutor_.AddSubstitution(ast_enum.type_dependencies[i].name, expected_.parameters[i]);
    }
    if (!matches) {
      substitutor_.PopScope();
      continue;
    }
    for (const auto &constructor : rule.outputs) {
      // Skip if this is not the constructor we are looking for
      if (constructor.identifier.name != val.constructor_identifier.name) {
        continue;
      }
      DLOG(INFO) << "Found constructor " << constructor.identifier.name << ", checking fields";
      return CheckConstructedValue(val, constructor);
    }
  }
  return Error(
      CreateError() << "Constructor \"" << val.constructor_identifier.name << "\" cannot be used in this context at "
                    << val.location);
}

std::optional<Error>
TypeComparator::CheckConstructedValue(const ast::ConstructedValue &val, const ast::TypeWithFields &constructor) {
  // Check that all fields have the correct types
  for (size_t i = 0; i < constructor.fields.size(); ++i) {
    const auto &field         = constructor.fields[i];
    const auto &expected_type = std::get<ast::TypeExpression>(substitutor_(field.type_expression));
    DLOG(INFO) << "Checking that field " << field.name << " has type " << expected_type;
    if (std::holds_alternative<ast::VarAccess>(*val.fields[i].second)) {
      const auto &var_access = std::get<ast::VarAccess>(*val.fields[i].second);
      context_.back()->AddName(var_access.var_identifier.name, expected_type);
      continue;
    }
    auto field_err =
        TypeComparator(expected_type, ast_, &context_, &substitutor_, &z3_stuff_).Compare(*val.fields[i].second);
    if (field_err) {
      DLOG(ERROR) << "Field " << field.name << " has incorrect type";
      return field_err;
    }
  }
  return {};
}

} // namespace dbuf::checker
