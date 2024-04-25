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
#include "core/checker/common.h"
#include "core/interning/interned_string.h"
#include "glog/logging.h"

#include <algorithm>
#include <cstddef>
#include <optional>
#include <ranges>
#include <sstream>
#include <unordered_set>
#include <variant>

namespace dbuf::checker {

std::optional<Error> NameResolutionChecker::operator()(const ast::AST &ast) {
  PushScope();
  if (!AddGlobalNames(ast)) {
    return error_;
  }
  InitConstructorFields(ast);

  for (const auto &[_, type] : ast.types) {
    if (!std::visit(*this, type)) {
      return error_;
    }
  }
  return {};
}

bool NameResolutionChecker::operator()(const ast::Message &ast_message) {
  PushScope();
  for (const auto &dependency : ast_message.type_dependencies) {
    if (!(*this)(dependency, true)) {
      return false;
    }
  }
  for (const auto &field : ast_message.fields) {
    if (!(*this)(field, false)) {
      return false;
    }
  }
  PopScope();
  return true;
}

bool NameResolutionChecker::operator()(const ast::Enum &ast_enum) {
  PushScope();
  for (const auto &dependency : ast_enum.type_dependencies) {
    if (!(*this)(dependency, true)) {
      return false;
    };
  }
  for (const auto &rule : ast_enum.pattern_mapping) {
    if (!(*this)(rule)) {
      return false;
    }
  }
  PopScope();
  return true;
}

bool NameResolutionChecker::operator()(const ast::Enum::Rule &rule) {
  PushScope();
  accept_aliases_ = true;
  for (const auto &input : rule.inputs) {
    if (!std::visit(*this, input)) {
      return false;
    }
  }
  accept_aliases_ = false;
  for (const auto &output : rule.outputs) {
    if (!(*this)(output)) {
      return false;
    }
  }
  PopScope();
  return true;
}

bool NameResolutionChecker::operator()(const ast::Constructor &constructor) {
  return std::ranges::all_of(constructor.fields.begin(), constructor.fields.end(), [this](auto field) {
    return (*this)(field, false);
  });
}

bool NameResolutionChecker::operator()(const ast::TypedVariable &variable, bool allow_shadowing) {
  if (!AddName(variable.name, "variable", allow_shadowing)) {
    return false;
  }
  return (*this)(variable.type_expression);
}

bool NameResolutionChecker::operator()(const Field &field) {
  if (accept_aliases_ && std::holds_alternative<ast::VarAccess>(*field.second) &&
      std::get<ast::VarAccess>(*field.second).field_identifiers.empty()) {
    auto var_access = std::get<ast::VarAccess>(*field.second);
    if (!AddName(var_access.var_identifier.name, "variable", false)) {
      return false;
    }
  }
  return std::visit(*this, *field.second);
}

bool NameResolutionChecker::operator()(const ast::Value &value) {
  return std::visit(*this, value);
}

bool NameResolutionChecker::operator()(const ast::ConstructedValue &value) {
  if (!IsInScope(value.constructor_identifier.name)) {
    error_ = Error {"Undefined constructor: \"" + value.constructor_identifier.name.GetString() + "\""};
    return false;
  }
  if (constructor_to_fields_[value.constructor_identifier.name].size() != value.fields.size()) {
    error_ = Error (CreateError() << "Expected " << constructor_to_fields_[value.constructor_identifier.name].size() << " parameters, but got " << value.fields.size() << " in ConstructedValue " << value);
    return false;
  }
  for (size_t i = 0; i < value.fields.size(); ++i) {
    if (constructor_to_fields_[value.constructor_identifier.name][i] != value.fields[i].first.name) {
      error_ = Error(CreateError() << "Expected field " << constructor_to_fields_[value.constructor_identifier.name][i] << ", but got " << value.fields[i].first.name << " in ConstructedValue " << value);
      return false;
    }
    if (!(*this)(value.fields[i])) {
      return false;
    }
  }
  return true;
}

bool NameResolutionChecker::operator()(const ast::CollectionValue & /* value */) {
  return true;
}

bool NameResolutionChecker::operator()(const ast::Star & /* value */) {
  return true;
}

bool NameResolutionChecker::operator()(const ast::BinaryExpression &expr) {
  return (std::visit(*this, *(expr.left)) && std::visit(*this, *(expr.right)));
}

bool NameResolutionChecker::operator()(const ast::UnaryExpression &expr) {
  return std::visit(*this, *(expr.expression));
}

bool NameResolutionChecker::operator()(const ast::TypeExpression &expr) {
  if (!IsInScope(expr.identifier.name)) {
    error_ = Error(
        CreateError() << "Undefined type name: \"" << expr.identifier.name.GetString() << "\" at "
                      << expr.identifier.location);
    return false;
  }
  return std::ranges::all_of(expr.parameters.begin(), expr.parameters.end(), [this](auto parameter) {
    return std::visit(*this, *parameter);
  });
}

bool NameResolutionChecker::operator()(const ast::VarAccess &var_access) {
  if (!IsInScope(var_access.var_identifier.name)) {
    error_ = Error(
        CreateError() << "Undefined variable: \"" << var_access.var_identifier.name.GetString() << "\" at "
                      << var_access.var_identifier.location);
    return false;
  }
  return true;
}

bool NameResolutionChecker::operator()(const ast::ArrayAccess &array_access) {
  return (std::visit(*this, *array_access.array_identifier) && std::visit(*this, *array_access.ind));
}

void NameResolutionChecker::PushScope() {
  scopes_.emplace_back();
}

void NameResolutionChecker::PopScope() {
  DCHECK(!scopes_.empty());
  scopes_.pop_back();
}

bool NameResolutionChecker::IsInScope(InternedString name) {
  return std::ranges::any_of(scopes_.begin(), scopes_.end(), [&name](auto &scope) { return scope.contains(name); });
}

bool NameResolutionChecker::AddName(InternedString name, std::string &&identifier_type, bool allow_shadowing) {
  DCHECK(!scopes_.empty());
  if ((!allow_shadowing) && IsInScope(name)) {
    error_ = Error {"Re-declaration of " + identifier_type + ": " + "\"" + name.GetString() + "\""};
    return false;
  }
  scopes_.back().insert(name);
  return true;
}

void NameResolutionChecker::AddFields(const InternedString &constructor_name, const ast::TypeWithFields &type) {
  for (const auto &field : type.fields) {
    constructor_to_fields_[constructor_name].emplace_back(field.name);
  }
}

bool NameResolutionChecker::AddGlobalNames(const ast::AST &ast) {
  if (!AddName(InternedString("Int"), "type", false) || !AddName(InternedString("Unsigned"), "type", false) ||
      !AddName(InternedString("String"), "type", false) || !AddName(InternedString("Float"), "type", false) ||
      !AddName(InternedString("Bool"), "type", false) || !AddName(InternedString("Array"), "type", false) ||
      !AddName(InternedString("Set"), "type", false)) {
    return false;
  };

  auto visitor = [this](const auto &type) -> bool {
    using T = std::decay_t<decltype(type)>;
    if constexpr (std::is_same_v<T, ast::Message>) {
      return AddName(type.identifier.name, "message", false);
    } else if constexpr (std::is_same_v<T, ast::Enum>) {
      if (!AddName(type.identifier.name, "enum", false)) {
        return false;
      }
      return std::ranges::all_of(type.pattern_mapping.begin(), type.pattern_mapping.end(), [this](auto &pattern) {
        return std::ranges::all_of(pattern.outputs.begin(), pattern.outputs.end(), [this](auto &constructor) {
          return AddName(constructor.identifier.name, "constructor", false);
        });
      });
    }
  };

  for (const auto &[_, type] : ast.types) {
    if (!std::visit(visitor, type)) {
      return false;
    }
  }
  return true;
}

void NameResolutionChecker::InitConstructorFields(const ast::AST &ast) {
  auto visitor = [this](const auto &type) -> void {
    using T = std::decay_t<decltype(type)>;
    if constexpr (std::is_same_v<T, ast::Message>) {
      AddFields(type.identifier.name, type);
    } else if constexpr (std::is_same_v<T, ast::Enum>) {
      for (const auto &pattern : type.pattern_mapping) {
        for (const auto &constructor : pattern.outputs) {
          AddFields(constructor.identifier.name, constructor);
        }
      }
    }
  };

  for (const auto &[_, type_variant] : ast.types) {
    std::visit(visitor, type_variant);
  }
}

} // namespace dbuf::checker
