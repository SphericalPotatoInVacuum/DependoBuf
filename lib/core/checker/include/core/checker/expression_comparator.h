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
#include "glog/logging.h"
#include "location.hh"
#include "z3++.h"

#include <sstream>
#include <unordered_map>
#include <vector>

namespace dbuf::checker {

struct Z3stuff {
  explicit Z3stuff();

  using NameToSort           = std::unordered_map<InternedString, z3::sort>;
  using NameToCollectionSort = std::unordered_map<InternedString, NameToSort>;
  using FieldToFuncDecl      = std::unordered_map<InternedString, z3::func_decl>;
  using NameToFields         = std::unordered_map<InternedString, FieldToFuncDecl>;

  z3::context context_;
  z3::solver solver_;
  NameToSort sorts_;
  NameToCollectionSort collection_sorts_;
  FieldToFuncDecl constructors_;
  NameToFields accessors_;
};

struct ExpressionToZ3 {
  Z3stuff &z3_stuff;
  const ast::AST &ast;
  std::deque<Scope *> &context;
  std::unordered_map<InternedString, z3::expr> var_to_expr = {};

  z3::expr operator()(const ast::Expression &expression);

  z3::expr operator()(const ast::BinaryExpression &binary_expression);
  z3::expr operator()(const ast::UnaryExpression &unary_expression);
  z3::expr operator()(const ast::TypeExpression & /* type_expression */);
  z3::expr operator()(const ast::VarAccess &var_access);
  z3::expr operator()(const ast::ArrayAccess & /*value*/);

  z3::expr operator()(const ast::Value &value);

  z3::expr operator()(const ast::ScalarValue<bool> &value);
  z3::expr operator()(const ast::ScalarValue<int64_t> &value);
  z3::expr operator()(const ast::ScalarValue<uint64_t> &value);
  z3::expr operator()(const ast::ScalarValue<double> &value);
  z3::expr operator()(const ast::ScalarValue<std::string> &value);
  z3::expr operator()(const ast::ConstructedValue &value);
  z3::expr operator()(const ast::CollectionValue & /*value*/);
};

[[nodiscard]] std::optional<Error> CompareExpressions(
    const ast::Expression &expected,
    const ast::Expression &got,
    Z3stuff &z3_stuff,
    const ast::AST &ast,
    std::deque<Scope *> &context);

} // namespace dbuf::checker
