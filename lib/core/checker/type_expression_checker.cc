#include "core/checker/type_expression_checker.h"

#include "core/ast/ast.h"
#include "core/ast/expression.h"
#include "core/interning/interned_string.h"
#include "location.hh"

#include <string>
#include <variant>
#include <vector>

namespace dbuf::checker {

TypeExpressionChecker::TypeExpressionChecker(const ast::AST &ast, const std::vector<InternedString> &sorted_graph)
    : ast_(ast)
    , sorted_graph_(sorted_graph)
    , z3_sorts_(
          {{InternedString("Int"), z3_context_.int_sort()},
           {InternedString("Unsigned"), z3_context_.int_sort()},
           {InternedString("Book"), z3_context_.bool_sort()},
           {InternedString("String"), z3_context_.string_sort()}}) {}

void TypeExpressionChecker::BuildConstructorToEnum() {
  for (const auto &ast_enum : ast_.enums) {
    for (const auto &rule : ast_enum.second.pattern_mapping) {
      for (const auto &constructor : rule.outputs) {
        constructor_to_enum_[constructor.identifier.name] = ast_enum.second.identifier.name;
      }
    }
  }
}

void TypeExpressionChecker::PushScope() {
  context_.emplace_back();
}

void TypeExpressionChecker::PopScope() {
  if (context_.empty()) {
    throw std::logic_error("Can't delete scope from empty scopes.");
  }
  context_.pop_back();
}

bool TypeExpressionChecker::IsInScope(InternedString name) {
  return std::ranges::any_of(context_.begin(), context_.end(), [&name](auto &scope) { return scope.contains(name); });
}

void TypeExpressionChecker::CheckTypes() {
  BuildConstructorToEnum();

  for (const auto &node : sorted_graph_) {
    // Checking message
    if (ast_.messages.contains(node)) {
      // Scope of the message checked
      PushScope();
      // Iterator to message from ast map
      const ast::Message ast_message = ast_.messages.at(node);

      z3_sorts_.emplace(
          ast_message.identifier.name,
          z3_context_.uninterpreted_sort(ast_message.identifier.name.GetString().c_str()));

      std::vector<z3::sort> dependency_sorts;

      // First step is to check if dependencies' types are correctly constructed
      for (const auto &dependency : ast_message.type_dependencies) {
        CheckTypeExpression(dependency.type_expression);

        // After we checked the depdency we can add it to scope to be seen by other dependencies
        AddName(dependency.name, dependency.type_expression);

        dependency_sorts.push_back(z3_sorts_.at(dependency.type_expression.identifier.name));
      }

      std::vector<z3::sort> field_sorts;

      // Same with fields
      for (const auto &field : ast_message.fields) {
        CheckTypeExpression(field.type_expression);

        // After we checked the depdency we can add it to scope to be seen by other dependencies
        AddName(field.name, field.type_expression);

        field_sorts.push_back(z3_sorts_.at(field.type_expression.identifier.name));
      }

      std::vector<z3::sort> constructor_parameter_sorts(dependency_sorts);
      constructor_parameter_sorts.insert(constructor_parameter_sorts.end(), field_sorts.begin(), field_sorts.end());

      z3_constructors_.emplace(
          ast_message.identifier.name,
          z3_context_.function(
              ast_message.identifier.name.GetString().c_str(),
              static_cast<size_t>(constructor_parameter_sorts.size()),
              constructor_parameter_sorts.data(),
              z3_sorts_.at(ast_message.identifier.name)));

      // Clear used scope
      PopScope();
    } else {
      // TODO(alisa-vernigor): Check enum
    }
  }
}

void TypeExpressionChecker::CheckTypeExpression(const ast::TypeExpression &type_expression) {
  auto messages_it                         = ast_.messages.find(type_expression.identifier.name);
  ast::DependentType const *dependent_type = nullptr;
  if (messages_it != ast_.messages.end()) {
    dependent_type = &messages_it->second;
  } else {
    auto enums_it  = ast_.enums.find(type_expression.identifier.name);
    dependent_type = &messages_it->second;
  }
  // Number of parameters should be equal to number of dependencies of message/enum
  if (dependent_type->type_dependencies.size() != type_expression.parameters.size()) {
    errors_.emplace_back(Error {
        .message = "Expected " + std::to_string(dependent_type->type_dependencies.size()) +
                   " parameters for typename \"" + type_expression.identifier.name.GetString() + "\", but got " +
                   std::to_string(type_expression.parameters.size())});
    return;
  }

  // Next step is to check parameters
  CheckTypeDependencies(dependent_type->type_dependencies, type_expression.parameters);
}

void TypeExpressionChecker::AddName(InternedString name, ast::TypeExpression type) {
  if (context_.empty()) {
    throw std::logic_error("Can't add name to empty scopes.");
  }

  context_.back()[name] = std::move(type);
}

void TypeExpressionChecker::operator()(const ast::TypeExpression &expected_type, const ast::ConstructedValue &value) {
  InternedString enum_identifier = constructor_to_enum_[value.constructor_identifier.name];

  if (enum_identifier != expected_type.identifier.name) {
    errors_.emplace_back(Error {
        .message = "Got value of type \"" + enum_identifier.GetString() + "\", but expected type is \"" +
                   expected_type.identifier.name.GetString() + "\""});
    return;
  }

  // TODO(alisa-vernigor): The pattern matching problem must be solved to check that the
  // constructor used creates the required type.
}

void TypeExpressionChecker::operator()(
    const std::vector<ast::TypedVariable> &type_dependencies,
    const ast::TypeExpression &type_expression) {
  if (type_expression.parameters.size() != type_dependencies.size()) {
    errors_.emplace_back(Error {
        .message = "Expected " + std::to_string(type_dependencies.size()) + " parameters for typename \"" +
                   type_expression.identifier.name.GetString() + "\", but got " +
                   std::to_string(type_expression.parameters.size())});
    return;
  }

  for (size_t id = 0; id < type_dependencies.size(); ++id) {
    CompareExpressions(type_dependencies[id].type_expression, *type_expression.parameters[id]);
  }
}

void TypeExpressionChecker::operator()(const ast::TypeExpression &expected_type, const ast::VarAccess &expression) {
  // Case: does foo has type Foo?
  if (expression.field_identifiers.empty()) {
    // We can find foo in scope and just compare its type
    CompareTypeExpressions(expected_type, context_.back()[expression.var_identifier.name]);
    return;
  }
  // Else we have case: does foo.bar.buzz has type Buzz?

  // First lets find typename of foo. We have foo in scope, so we can get it from there
  InternedString message_name = context_.back()[expression.var_identifier.name].identifier.name;
  // The field we are looking for. In case foo.bar.buzz we are looking for bar
  InternedString expected_field = expression.field_identifiers[0].name;

  bool found = false;

  // Let's notice the Type(foo.bar.buzz) == Type(bar.buzz) == Type (buzz)
  // We want to check type recursively, so I add new scope -- the scope of message Type(foo)
  PushScope();
  for (const auto &dependencie : ast_.messages.at(message_name).type_dependencies) {
    AddName(dependencie.name, dependencie.type_expression);
  }
  for (const auto &field : ast_.messages.at(message_name).fields) {
    AddName(field.name, field.type_expression);
    if (field.name == expected_field) {
      found = true;
      break;
    }
  }

  // This case is not checked in name resolution checker, so I do it there
  if (!found) {
    errors_.emplace_back(Error {
        .message =
            "Field \"" + expected_field.GetString() + "\" not found in message \"" + message_name.GetString() + "\""});
    return;
  }

  // Recursion
  ast::VarAccess var_access;
  var_access.var_identifier    = {parser::location(), expected_field};
  var_access.field_identifiers = std::vector<ast::Identifier>();
  for (size_t id = 1; id < expression.field_identifiers.size(); ++id) {
    var_access.field_identifiers.push_back(expression.field_identifiers[id]);
  }

  (*this)(expected_type, var_access);

  PopScope();
}

void TypeExpressionChecker::CompareTypeExpressions(
    const ast::TypeExpression &expected_type,
    const ast::TypeExpression &expression) {
  if (expected_type.identifier.name != expression.identifier.name) {
    errors_.emplace_back(Error {
        .message = "Got type \"" + expression.identifier.name.GetString() + "\", but expected type is \"" +
                   expected_type.identifier.name.GetString() + "\""});
    return;
  }

  if (expected_type.parameters.size() != expression.parameters.size()) {
    errors_.emplace_back(Error {
        .message = "Expected " + std::to_string(expected_type.parameters.size()) + "type parametes, but got " +
                   std::to_string(expression.parameters.size())});
    return;
  }

  for (size_t id = 0; id < expected_type.parameters.size(); ++id) {
    CompareExpressions(*expected_type.parameters[id], *expression.parameters[id]);
  }
}

void TypeExpressionChecker::CheckTypeDependencies(
    const std::vector<ast::TypedVariable> &type_dependencies,
    const std::vector<std::shared_ptr<const ast::Expression>> &type_parameters) {
  substitutor_.ClearSubstitutionMap();
  for (size_t id = 0; id < type_dependencies.size(); ++id) {
    // Update type using already known substitutions
    ast::Expression substituted_type = substitutor_(type_dependencies[id].type_expression);

    // Compare types
    CompareExpressions(substituted_type, *type_parameters[id]);

    // Now we can update substitutions
    substitutor_.AddSubstitution(type_dependencies[id].name, type_parameters[id]);
  }
}

bool TypeExpressionChecker::IsTypeName(InternedString name) const {
  return ast_.enums.contains(name) || ast_.messages.contains(name);
}

void TypeExpressionChecker::CompareExpressions(const ast::Expression & /*expected*/, const ast::Expression & /*got*/) {
  // TODO(SphericalPotatoInVacuum): compare expressions
}

} // namespace dbuf::checker
