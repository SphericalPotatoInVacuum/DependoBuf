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
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace dbuf::checker {

struct NameResolutionChecker {
  ErrorList operator()(const ast::AST &ast);

  void operator()(const ast::Message &ast_message);

  void operator()(const ast::Enum &ast_enum);

  void operator()(const ast::Enum::Rule &rule);

  void operator()(const ast::Constructor &constructor);

  void operator()(const ast::TypedVariable &variable, bool allow_shadowing);

private:
  using Field = std::pair<ast::Identifier, std::shared_ptr<const ast::Expression>>;

public:
  void operator()(const Field &field);

  template <typename T>
  void operator()(const ast::ScalarValue<T> &) {}
  void operator()(const ast::ConstructedValue &value);
  void operator()(const ast::Value &value);

  void operator()(const ast::Star &value);

  void operator()(const ast::BinaryExpression &expr);
  void operator()(const ast::UnaryExpression &expr);
  void operator()(const ast::TypeExpression &expr);
  void operator()(const ast::VarAccess &var_access);

private:
  using Scope                = std::unordered_set<InternedString>;
  using ConstructorFields    = std::unordered_set<InternedString>;
  using ConstructorFieldsMap = std::unordered_map<InternedString, ConstructorFields>;

  // private members
  ErrorList errors_;

  std::deque<Scope> scopes_;

  ConstructorFieldsMap constructor_to_fields_;

  // For pattern matching only. Case: Succ {prev: n}, where n is alias
  bool accept_aliases_ {false};

  // private methods
  bool IsInScope(InternedString name);
  void AddName(InternedString name, std::string &&identifier_type, bool allow_shadowing);

  void InitConstructorFields(const ast::AST &ast);
  void AddFields(const InternedString &constructor_name, const ast::TypeWithFields &type);

  void AddGlobalNames(const ast::AST &ast);

  void PushScope();
  void PopScope();
};

} // namespace dbuf::checker
