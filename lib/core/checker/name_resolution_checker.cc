/*
This file is part of DependoBuf project.

Copyright (C) 2023 Alexander Bogdanov, Alice Vernigor

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/
#include "core/checker/name_resolution_checker.h"

#include "core/ast/ast.h"
#include "core/ast/expression.h"
#include "core/interning/interned_string.h"
#include "glog/logging.h"

#include <algorithm>
#include <sstream>
#include <variant>

namespace dbuf::checker {

ErrorList NameResolutionChecker::operator()(const ast::AST &ast) {
  PushScope();
  AddGlobalNames(ast);
  InitConstructorFields(ast);

  for (const auto &[_, type] : ast.types) {
    std::visit(*this, type);
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
  accept_aliases_ = true;
  for (const auto &input : rule.inputs) {
    std::visit(*this, input);
  }
  accept_aliases_ = false;

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
  if (accept_aliases_ && std::holds_alternative<ast::VarAccess>(*field.second) &&
      std::get<ast::VarAccess>(*field.second).field_identifiers.empty()) {
    AddName(std::get<ast::VarAccess>(*field.second).var_identifier.name, "variable", true);
    return;
  }
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
    if (!constructor_to_fields_[value.constructor_identifier.name].contains(field.first.name)) {
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
  DCHECK(!scopes_.empty());

  if ((!allow_shadowing) && IsInScope(name)) {
    errors_.push_back({"Re-declaration of " + identifier_type + ": " + "\"" + name.GetString() + "\""});
  }

  scopes_.back().insert(name);
}

void NameResolutionChecker::InitConstructorFields(const ast::AST &ast) {
  auto visitor = [this](const auto &type) {
    using T = std::decay_t<decltype(type)>;
    if constexpr (std::is_same_v<T, ast::Enum>) {
      for (const auto &pattern : type.pattern_mapping) {
        for (const auto &constructor : pattern.outputs) {
          AddFields(constructor.identifier.name, constructor);
        }
      }
    } else if constexpr (std::is_same_v<T, ast::Message>) {
      AddFields(type.identifier.name, type);
    }
  };
  for (const auto &[type_name, type_variant] : ast.types) {
    std::visit(visitor, type_variant);
  }
}

void NameResolutionChecker::AddFields(const InternedString &constructor_name, const ast::TypeWithFields &type) {
  for (const auto &field : type.fields) {
    constructor_to_fields_[constructor_name].insert(field.name);
  }
}

void NameResolutionChecker::AddGlobalNames(const ast::AST &ast) {
  AddName(InternedString("Int"), "type", false);
  AddName(InternedString("Unsigned"),"type",false);
  AddName(InternedString("String"), "type", false);
  AddName(InternedString("Float"), "type", false);
  AddName(InternedString("Bool"), "type", false);
  AddName(InternedString("Array"), "type", false);
  AddName(InternedString("Set"), "type", false);

  auto visitor = [this](const auto &type) {
    if constexpr (std::is_same_v<std::decay_t<decltype(type)>, ast::Message>) {
      AddName(type.identifier.name, "message", false);
    } else if constexpr (std::is_same_v<std::decay_t<decltype(type)>, ast::Enum>) {
      AddName(type.identifier.name, "enum", false);
      for (const auto &pattern : type.pattern_mapping) {
        for (const auto &constructor : pattern.outputs) {
          AddName(constructor.identifier.name, "constructor", false);
        }
      }
    }
  };
  for (const auto &[_, type] : ast.types) {
    std::visit(visitor, type);
  }
}

void NameResolutionChecker::PushScope() {
  scopes_.emplace_back();
}

void NameResolutionChecker::PopScope() {
  DCHECK(!scopes_.empty());
  scopes_.pop_back();
}

} // namespace dbuf::checker
