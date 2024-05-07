/*
This file is part of DependoBuf project.

Copyright (C) 2023 Alexander Bogdanov, Alice Vernigor

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/
#pragma once

#include "core/ast/ast.h"
#include "core/ast/expression.h"
#include "core/checker/common.h"
#include "core/interning/interned_string.h"

#include <deque>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace dbuf::checker {

class NameResolutionChecker {
public:
  [[nodiscard]] std::optional<Error> operator()(const ast::AST &ast);

  bool operator()(const ast::Message &ast_message);
  bool operator()(const ast::Enum &ast_enum);
  bool operator()(const ast::Enum::Rule &rule);
  bool operator()(const ast::Func &ast_func);
  bool operator()(const ast::Constructor &constructor);
  bool operator()(const ast::TypedVariable &variable, bool allow_shadowing);

private:
  using Field = std::pair<ast::Identifier, ast::ExprPtr>;

public:
  bool operator()(const Field &field);

  bool operator()(const ast::Value &value);

  template <typename T>
  bool operator()(const ast::ScalarValue<T> &) {
    return true;
  }
  bool operator()(const ast::ConstructedValue &value);
  bool operator()(const ast::CollectionValue &value);

  bool operator()(const ast::Star &value);

  bool operator()(const ast::BinaryExpression &expr);
  bool operator()(const ast::UnaryExpression &expr);
  bool operator()(const ast::TypeExpression &expr);
  bool operator()(const ast::VarAccess &var_access);
  bool operator()(const ast::ArrayAccess &array_access);

private:
  using Scope                = std::unordered_set<InternedString>;
  using ConstructorFields    = std::vector<InternedString>;
  using ConstructorFieldsMap = std::unordered_map<InternedString, ConstructorFields>;

  std::optional<Error> error_;
  std::deque<Scope> scopes_;
  ConstructorFieldsMap constructor_to_fields_;

  // For pattern matching only. Case: Succ {prev: n}, where n is alias
  bool accept_aliases_ = false;

  void PushScope();
  void PopScope();
  bool IsInScope(InternedString name);

  bool AddName(InternedString name, std::string &&identifier_type, bool allow_shadowing);
  void AddFields(const InternedString &constructor_name, const ast::TypeWithFields &type);

  bool AddGlobalNames(const ast::AST &ast);
  void InitConstructorFields(const ast::AST &ast);
};

} // namespace dbuf::checker
