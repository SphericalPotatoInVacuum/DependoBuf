#pragma once

#include "core/checker/name_resolution_checker.h"

namespace dbuf::checker {

ErrorList NameResolutionChecker::operator()(const ast::AST &ast) {
  PushScope();
  AddGlobalNames(ast);

  constructors_fields_ = GetConstructorFields(ast);

  (*this)(ast.messages);

  (*this)(ast.enums);

  return errors_;
}

void NameResolutionChecker::operator()(const std::unordered_map<InternedString, ast::Enum> &enums) {
  for (const auto &ast_enum : enums) {
    (*this)(ast_enum.second);
  }
}

void NameResolutionChecker::operator()(const ast::Enum &ast_enum) {
  PushScope();
  if (!IsInScope(ast_enum.name)) {
    errors_.push_back({"Enum's name " + ast_enum.name.GetString() + " is already in scope."});
  }
  (*this)(ast_enum.type_dependencies);

  (*this)(ast_enum.pattern_mapping);
  PopScope();
}

void NameResolutionChecker::operator()(const std::vector<ast::Enum::Rule> &rules) {
  for (const auto &rule : rules) {
    (*this)(rule);
  }
}

void NameResolutionChecker::operator()(const ast::Enum::Rule &rule) {
  PushScope();

  isShadowing_ = true;

  (*this)(rule.inputs);

  isShadowing_ = false;

  (*this)(rule.outputs);

  PopScope();
}

void NameResolutionChecker::operator()(std::vector<ast::Constructor> &constructors) {
  for (const auto &constructor : constructors) {
    (*this)(constructor);
  }
}

void NameResolutionChecker::operator()(const ast::Constructor &constructor) {
  if (IsInScope(constructor.name)) {
    errors_.push_back(
        {"Constructor's name " + constructor.name.GetString() + " is already in scope."});
  }
  (*this)(constructor.fields);
}

void NameResolutionChecker::operator()(const ast::Star &) {}

void NameResolutionChecker::operator()(
    const std::unordered_map<InternedString, ast::Message> &messages) {
  for (const auto &message : messages) {
    (*this)(message.second);
  }
}

void NameResolutionChecker::operator()(const ast::Message &ast_message) {
  PushScope();
  if (IsInScope(ast_message.name)) {
    errors_.push_back({"Message's name " + ast_message.name.GetString() + " is already in scope."});
  }
  (*this)(ast_message.type_dependencies);
  (*this)(ast_message.fields);
  PopScope();
}

void NameResolutionChecker::operator()(const std::vector<ast::TypedVariable> &dependencies) {
  for (const auto &dependency : dependencies) {
    (*this)(dependency);
  }
}

void NameResolutionChecker::operator()(const ast::TypedVariable &typed_variable) {
  (*this)(typed_variable.type_expression);
  AddName(typed_variable.name);
}

void NameResolutionChecker::operator()(const ast::TypeExpression &type_expression) {
  if (!IsInScope(type_expression.name)) {
    errors_.push_back({"Type's name " + type_expression.name.GetString() + " is not in scope."});
  }

  (*this)(type_expression.parameters);
}

void NameResolutionChecker::operator()(
    const std::vector<std::unique_ptr<ast::Expression>> &expressions) {
  for (const auto &expression : expressions) {
    std::visit(*this, *expression);
  }
}

void NameResolutionChecker::operator()(const ast::ConstructedValue &value) {
  if (!IsInScope(value.constructor_identifier)) {
    errors_.push_back(
        {"Constructor's identifier " + value.constructor_identifier.GetString() +
         " is not in scope."});
  }
  PushScope();
  (*this)(value.constructor_identifier, value.fields);
  PopScope();
}

void NameResolutionChecker::operator()(
    InternedString constructor_identifier,
    const std::vector<std::pair<InternedString, std::unique_ptr<ast::Expression>>> &fields) {
  for (const auto &field : fields) {
    if (!constructors_fields_[constructor_identifier].contains(field.first)) {
      errors_.push_back(
          {"No field with name " + field.first.GetString() + " in constructor " +
           constructor_identifier.GetString() + "."});
    }

    (*this)(field);
  }
}

void NameResolutionChecker::operator()(
    const std::pair<InternedString, std::unique_ptr<ast::Expression>> &field) {
  std::visit(*this, *field.second);
  AddName(field.first);
}

void NameResolutionChecker::operator()(const ast::BinaryExpression &expr) const {
  std::visit(*this, *(expr.left));
  std::visit(*this, *expr.right);
}
void NameResolutionChecker::operator()(const ast::UnaryExpression &expr) const {
  std::visit(*this, *(expr.expression));
}

void NameResolutionChecker::operator()(const ast::VarAccess &var_access) {
  if (IsInScope(var_access.var_identifier)) {
    errors_.push_back(
        {"Variable's identifier " + var_access.var_identifier.GetString() +
         " is already in scope."});
  }
}

bool NameResolutionChecker::IsInScope(InternedString name) {
  return std::ranges::any_of(scopes_.begin(), scopes_.end(), [&name](auto &scope) {
    return scope.contains(name);
  });
}

void NameResolutionChecker::AddName(InternedString name) {
  if (scopes_.empty()) {
    throw std::logic_error("Can't add name to empty scopes.");
  }

  if ((!isShadowing_) && IsInScope(name)) {
    errors_.push_back({"Name" + name.GetString() + " is already in scope."});
  }

  scopes_.back().insert(name);
}

std::unordered_map<InternedString, std::unordered_set<InternedString>>
NameResolutionChecker::GetConstructorFields(const ast::AST &ast) {
  std::unordered_map<InternedString, std::unordered_set<InternedString>>
      constructor_to_fields_names;

  for (const auto &ast_enum : ast.enums) {
    for (const auto &pattern : ast_enum.second.pattern_mapping) {
      for (const auto &constructor : pattern.outputs) {
        for (const auto &field : constructor.fields) {
          constructor_to_fields_names[constructor.name].insert(field.name);
        }
      }
    }
  }

  return constructor_to_fields_names;
}

void NameResolutionChecker::AddGlobalNames(const ast::AST &ast) {
  AddName(InternedString("Int"));
  AddName(InternedString("String"));
  AddName(InternedString("Float"));

  for (const auto &ast_message : ast.messages) {
    AddName(ast_message.first);
  }

  for (const auto &ast_enum : ast.enums) {
    AddName(ast_enum.first);

    for (const auto &pattern : ast_enum.second.pattern_mapping) {
      for (const auto &constructor : pattern.outputs) {
        AddName(constructor.name);
      }
    }
  }
}

void NameResolutionChecker::PushScope() {
  scopes_.emplace_back();
}

void NameResolutionChecker::PopScope() {
  if (scopes_.empty()) {
    throw std::logic_error("Can't delete scope from empty scopes.");
  }
  scopes_.pop_back();
}

} // namespace dbuf::checker