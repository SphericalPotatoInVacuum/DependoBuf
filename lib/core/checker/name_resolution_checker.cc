#include "core/checker/name_resolution_checker.h"

#include <algorithm>

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
    PushScope();
    (*this)(ast_enum.second.type_dependencies);

    (*this)(ast_enum.second.pattern_mapping);
    PopScope();
  }
}

void NameResolutionChecker::operator()(const std::vector<ast::Enum::Rule> &rules) {
  for (const auto &rule : rules) {
    PushScope();

    isShadowing_ = true;

    (*this)(rule.inputs);

    isShadowing_ = false;

    (*this)(rule.outputs);

    PopScope();
  }
}

void NameResolutionChecker::operator()(const std::vector<ast::Enum::Rule::InputPattern> &inputs) {
  for (const auto &input : inputs) {
    std::visit(*this, input);
  }
}

void NameResolutionChecker::operator()(const ast::Value &value) {
  std::visit(*this, value);
}

void NameResolutionChecker::operator()(const std::vector<ast::Constructor> &constructors) {
  for (const auto &constructor : constructors) {
    (*this)(constructor.fields);
  }
}

void NameResolutionChecker::operator()(const ast::Star &) {}

void NameResolutionChecker::operator()(
    const std::unordered_map<InternedString, ast::Message> &messages) {
  for (const auto &ast_message : messages) {
    PushScope();
    (*this)(ast_message.second.type_dependencies);
    (*this)(ast_message.second.fields);
    PopScope();
  }
}

void NameResolutionChecker::operator()(const std::vector<ast::TypedVariable> &typed_variables) {
  for (const auto &typed_variable : typed_variables) {
    (*this)(typed_variable);
  }
}

void NameResolutionChecker::operator()(const ast::TypedVariable &typed_variable) {
  (*this)(typed_variable.type_expression);
  AddName(typed_variable.name, "variable");
}

void NameResolutionChecker::operator()(const ast::TypeExpression &type_expression) {
  if (!IsInScope(type_expression.name)) {
    errors_.push_back({"Undefined type name: \"" + type_expression.name.GetString() + "\""});
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
        {"Undefined constructor: \"" + value.constructor_identifier.GetString() + "\""});
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
  AddName(field.first, "field");
}

void NameResolutionChecker::operator()(const ast::BinaryExpression &expr) {
  std::visit(*this, *(expr.left));
  std::visit(*this, *expr.right);
}
void NameResolutionChecker::operator()(const ast::UnaryExpression &expr) {
  std::visit(*this, *(expr.expression));
}

void NameResolutionChecker::operator()(const ast::VarAccess &var_access) {
  if (!IsInScope(var_access.var_identifier)) {
    errors_.push_back({"Undefined variable: \"" + var_access.var_identifier.GetString() + "\""});
  }
}

bool NameResolutionChecker::IsInScope(InternedString name) {
  return std::ranges::any_of(scopes_.begin(), scopes_.end(), [&name](auto &scope) {
    return scope.contains(name);
  });
}

void NameResolutionChecker::AddName(InternedString name, std::string &&identifier_type) {
  if (scopes_.empty()) {
    throw std::logic_error("Can't add name to empty scopes.");
  }

  if ((!isShadowing_) && IsInScope(name)) {
    errors_.push_back(
        {"Re-declaration of " + identifier_type + ": " + "\"" + name.GetString() + "\""});
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
  AddName(InternedString("Int"), "type");
  AddName(InternedString("String"), "type");
  AddName(InternedString("Float"), "type");
  AddName(InternedString("Bool"), "type");

  for (const auto &ast_message : ast.messages) {
    AddName(ast_message.first, "message");
  }

  for (const auto &ast_enum : ast.enums) {
    AddName(ast_enum.first, "enum");
    for (const auto &pattern : ast_enum.second.pattern_mapping) {
      for (const auto &constructor : pattern.outputs) {
        AddName(constructor.name, "constructor");
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
