#include "type_expression_checker.h"

#include "core/ast/ast.h"

#include <string>
#include <vector>

namespace dbuf::checker {

TypeExpressionChecker::TypeExpressionChecker(ast::AST &ast)
    : ast_(ast) {}

void TypeExpressionChecker::GetConstructorToEnum() {
  for (const auto &ast_enum : ast_.enums) {
    for (const auto &rule : ast_enum.second.pattern_mapping) {
      for (const auto &constructor : rule.outputs) {
        constructor_to_enum_[constructor.name] = ast_enum.second.name;
      }
    }
  }
}

void TypeExpressionChecker::PushScope() {
  scopes_.emplace_back();
}

void TypeExpressionChecker::PopScope() {
  if (scopes_.empty()) {
    throw std::logic_error("Can't delete scope from empty scopes.");
  }
  scopes_.pop_back();
}

bool TypeExpressionChecker::IsInScope(InternedString name) {
  return std::ranges::any_of(scopes_.begin(), scopes_.end(), [&name](auto &scope) {
    return scope.contains(name);
  });
}

void TypeExpressionChecker::AddName(InternedString name, InternedString type) {
  if (scopes_.empty()) {
    throw std::logic_error("Can't add name to empty scopes.");
  }

  scopes_.back()[name] = type;
}

void TypeExpressionChecker::operator()(
    const ast::TypeExpression &expected_type,
    const ast::VarAccess &expression) {
  if (!IsInScope(expression.var_identifier)) {
    errors_.emplace_back(
        Error {.message = "Undefined variable: \"" + expression.var_identifier.GetString() + "\""});
    return;
  }

  if (expression.field_identifiers.empty()) {
    operator()(expected_type, scopes_.back()[expression.var_identifier]);
    return;
  }

  InternedString message_name   = scopes_.back()[expression.var_identifier];
  InternedString expected_field = expression.field_identifiers[0];
  bool found                    = false;
  PushScope();
  for (auto &dependencie : ast_.messages[message_name].type_dependencies) {
    AddName(dependencie.name, dependencie.type_expression.name);
    if (dependencie.name == expected_field) {
      found = true;
      break;
    }
  }

  if (!found) {
    for (auto &field : ast_.messages[message_name].fields) {
      AddName(field.name, field.type_expression.name);
      if (field.name == expected_field) {
        found = true;
        break;
      }
    }
  }

  ast::VarAccess var_access;
  var_access.var_identifier    = expected_field;
  var_access.field_identifiers = std::vector<InternedString>();
  for (size_t id = 1; id < expression.field_identifiers.size(); ++id) {
    var_access.field_identifiers.push_back(expression.field_identifiers[id]);
  }

  (*this)(expected_type, var_access);

  PopScope();
}

void TypeExpressionChecker::operator()(
    const ast::TypeExpression &expected_type,
    const ast::TypeExpression &expression) {
  if (expected_type.name != expression.name) {
    errors_.emplace_back(Error {
        .message = "Got type \"" + expression.name.GetString() + "\", but expected type is \"" +
                   expected_type.name.GetString() + "\""});
    return;
  }

  if (expected_type.parameters.size() != expression.parameters.size()) {
    errors_.emplace_back(Error {
        .message = "Expected " + std::to_string(expected_type.parameters.size()) +
                   "type parametes, but got " + std::to_string(expression.parameters.size())});
    return;
  }

  for (size_t id = 0; id < expected_type.parameters.size(); ++id) {
    (*this)(*expected_type.parameters[id], *expression.parameters[id]);
  }
}

void TypeExpressionChecker::operator()(
    const ast::TypeExpression &expected_type,
    const ast::ConstructedValue &value) {
  InternedString enum_identifier = constructor_to_enum_[value.constructor_identifier];

  if (enum_identifier != expected_type.name) {
    errors_.emplace_back(Error {
        .message = "Got value of type \"" + enum_identifier.GetString() +
                   "\", but expected type is \"" + expected_type.name.GetString() + "\""});
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
        .message = "Expected " + std::to_string(type_dependencies.size()) +
                   " parameters for typename \"" + type_expression.name.GetString() +
                   "\", but got " + std::to_string(type_expression.parameters.size())});
    return;
  }

  for (size_t id = 0; id < type_dependencies.size(); ++id) {
    (*this)(type_dependencies[id].type_expression, type_expression.parameters[id]);
  }
}

void TypeExpressionChecker::operator()(
    const std::unordered_map<InternedString, ast::Message> &messages) {
  for (const auto &ast_message : messages) {
    PushScope();
    for (const auto &dependencie : ast_message.second.type_dependencies) {
      (*this)(dependencie.type_expression);

      AddName(dependencie.name, dependencie.type_expression.name);
    }

    for (const auto &field : ast_message.second.fields) {
      (*this)(field.type_expression);

      AddName(field.name, field.type_expression.name);
    }
    PopScope();
  }
}
void TypeExpressionChecker::operator()(const std::unordered_map<InternedString, ast::Enum> &enums) {
  for (const auto &ast_enum : enums) {
    PushScope();
    for (const auto &dependencie : ast_enum.second.type_dependencies) {
      (*this)(dependencie.type_expression);

      AddName(dependencie.name, dependencie.type_expression.name);
    }

    for (const auto &pattern : ast_enum.second.pattern_mapping) {
      for (const auto &output : pattern.outputs) {
        for (const auto &field : output.fields) {
          (*this)(field.type_expression);
          AddName(field.name, field.type_expression.name);
        }
      }
    }
    PopScope();
  }
}

void TypeExpressionChecker::operator()() {
  GetConstructorToEnum();

  (*this)(ast_.messages);
  (*this)(ast_.enums);
}

void TypeExpressionChecker::operator()(const ast::TypeExpression &type_expression) {
  if (!IsTypeName(type_expression.name)) {
    errors_.emplace_back(
        Error {.message = "Unknown typename: \"" + type_expression.name.GetString() + "\""});
    return;
  }

  auto it_message = ast_.messages.find(type_expression.name);
  if (it_message != ast_.messages.end()) {
    (*this)(it_message->second.type_dependencies, type_expression);
  }

  auto it_enum = ast_.enums.find(type_expression.name);
  if (it_enum != ast_.enums.end()) {
    (*this)(it_enum->second.type_dependencies, type_expression);
  }
}

bool TypeExpressionChecker::IsTypeName(InternedString name) const {
  return ast_.enums.contains(name) || ast_.messages.contains(name);
}

} // namespace dbuf::checker