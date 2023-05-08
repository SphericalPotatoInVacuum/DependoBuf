#include "core/checker/name_resolution_checker.h"

#include <algorithm>
#include <sstream>

namespace dbuf::checker {

ErrorList NameResolutionChecker::operator()(const ast::AST &ast) {
  PushScope();
  AddGlobalNames(ast);

  constructors_fields_ = GetConstructorFields(ast);

  for (const auto &[message_name, message_body] : ast.messages) {
    (*this)(message_body);
  }

  for (const auto &[enum_name, enum_body] : ast.enums) {
    (*this)(enum_body);
  }

  return errors_;
}

void NameResolutionChecker::operator()(const ast::Message &ast_message) {
  PushScope();
  for (const auto &dependency : ast_message.type_dependencies) {
    (*this)(dependency, true);
  }
  for (const auto &field : ast_message.fields) {
    (*this)(field, false);
  }
  PopScope();
}

void NameResolutionChecker::operator()(const ast::Enum &ast_enum) {
  PushScope();
  for (const auto &dependency : ast_enum.type_dependencies) {
    (*this)(dependency, true);
  }
  for (const auto &rule : ast_enum.pattern_mapping) {
    (*this)(rule);
  }
  PopScope();
}

void NameResolutionChecker::operator()(const ast::Enum::Rule &rule) {
  PushScope();
  for (const auto &input : rule.inputs) {
    std::visit(*this, input);
  }
  for (const auto &output : rule.outputs) {
    (*this)(output);
  }
  PopScope();
}

void NameResolutionChecker::operator()(const ast::Constructor &constructor) {
  for (const auto &field : constructor.fields) {
    (*this)(field, false);
  }
}

void NameResolutionChecker::operator()(const ast::TypedVariable &variable, bool allow_shadowing) {
  (*this)(variable.type_expression);
  AddName(variable.name, "variable", allow_shadowing);
}

void NameResolutionChecker::operator()(const Field &field) {
  std::visit(*this, *field.second);
  AddName(field.first.name, "field", true);
}

void NameResolutionChecker::operator()(const ast::ConstructedValue &value) {
  if (!IsInScope(value.constructor_identifier.name)) {
    errors_.emplace_back(
        Error {.message = "Undefined constructor: \"" + value.constructor_identifier.name.GetString() + "\""});
  }
  PushScope();

  for (const auto &field : value.fields) {
    if (!constructors_fields_[value.constructor_identifier.name].contains(field.first.name)) {
      errors_.emplace_back(Error {
          .message = "No field with name " + field.first.name.GetString() + " in constructor " +
                     value.constructor_identifier.name.GetString()});
    }

    (*this)(field);
  }
  PopScope();
}

void NameResolutionChecker::operator()(const ast::Value &value) {
  std::visit(*this, value);
}

void NameResolutionChecker::operator()(const ast::Star &) {}

void NameResolutionChecker::operator()(const ast::BinaryExpression &expr) {
  std::visit(*this, *(expr.left));
  std::visit(*this, *(expr.right));
}

void NameResolutionChecker::operator()(const ast::UnaryExpression &expr) {
  std::visit(*this, *(expr.expression));
}

void NameResolutionChecker::operator()(const ast::TypeExpression &expr) {
  if (!IsInScope(expr.identifier.name)) {
    std::stringstream ms;
    ms << "Undefined type name: \"" << expr.identifier.name.GetString() << "\" at " << expr.identifier.location;
    errors_.emplace_back(Error {.message = ms.str()});
  }

  for (const auto &parameter : expr.parameters) {
    std::visit(*this, *parameter);
  }
}

void NameResolutionChecker::operator()(const ast::VarAccess &var_access) {
  if (!IsInScope(var_access.var_identifier.name)) {
    std::stringstream ms;
    ms << "Undefined variable: \"" << var_access.var_identifier.name.GetString() << "\" at "
       << var_access.var_identifier.location;
    errors_.emplace_back(Error {.message = ms.str()});
  }
}

bool NameResolutionChecker::IsInScope(InternedString name) {
  return std::ranges::any_of(scopes_.begin(), scopes_.end(), [&name](auto &scope) { return scope.contains(name); });
}

void NameResolutionChecker::AddName(InternedString name, std::string &&identifier_type, bool allow_shadowing) {
  if (scopes_.empty()) {
    throw std::logic_error("Can't add name to empty scopes.");
  }

  if ((!allow_shadowing) && IsInScope(name)) {
    errors_.push_back({"Re-declaration of " + identifier_type + ": " + "\"" + name.GetString() + "\""});
  }

  scopes_.back().insert(name);
}

NameResolutionChecker::ConstructorFieldsMap NameResolutionChecker::GetConstructorFields(const ast::AST &ast) {
  NameResolutionChecker::ConstructorFieldsMap constructor_to_fields_names;

  for (const auto &ast_enum : ast.enums) {
    for (const auto &pattern : ast_enum.second.pattern_mapping) {
      for (const auto &constructor : pattern.outputs) {
        for (const auto &field : constructor.fields) {
          constructor_to_fields_names[constructor.identifier.name].insert(field.name);
        }
      }
    }
  }

  for (const auto &ast_message : ast.messages) {
    for (const auto &field : ast_message.second.fields) {
      constructor_to_fields_names[ast_message.first].insert(field.name);
    }
  }

  return constructor_to_fields_names;
}

void NameResolutionChecker::AddGlobalNames(const ast::AST &ast) {
  AddName(InternedString("Int"), "type", false);
  AddName(InternedString("String"), "type", false);
  AddName(InternedString("Float"), "type", false);
  AddName(InternedString("Bool"), "type", false);

  for (const auto &ast_message : ast.messages) {
    AddName(ast_message.first, "message", false);
  }

  for (const auto &ast_enum : ast.enums) {
    AddName(ast_enum.first, "enum", false);
    for (const auto &pattern : ast_enum.second.pattern_mapping) {
      for (const auto &constructor : pattern.outputs) {
        AddName(constructor.identifier.name, "constructor", false);
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
