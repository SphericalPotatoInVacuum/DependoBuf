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
#include <vector>

namespace dbuf::checker {

struct Z3stuff {
  explicit Z3stuff()
      : solver_(context_)
      , sorts_(
            {{InternedString("Int"), context_.int_sort()},
             {InternedString("Unsigned"), context_.int_sort()},
             {InternedString("Bool"), context_.bool_sort()},
             {InternedString("String"), context_.string_sort()},
             {InternedString("Float"), context_.fpa_sort(11, 53)}}) {}

  using NameToSort        = std::unordered_map<InternedString, z3::sort>;
  using NameToConstructor = std::unordered_map<InternedString, z3::func_decl>;
  using FieldToAccessor   = std::unordered_map<InternedString, z3::func_decl>;
  using NameToFields      = std::unordered_map<InternedString, FieldToAccessor>;

  z3::context context_;
  z3::solver solver_;
  NameToSort sorts_;
  NameToConstructor constructors_;
  NameToFields accessors_;
};

struct ExpressionToZ3 {
  Z3stuff &z3_stuff;
  const ast::AST &ast;
  std::deque<Scope *> &context;
  std::unordered_map<InternedString, z3::expr> var_to_expr = {};

  z3::expr operator()(const ast::Expression &expression) {
    return std::visit(*this, expression);
  }

  z3::expr operator()(const ast::BinaryExpression &binary_expression) {
    z3::expr left  = std::visit(*this, *binary_expression.left);
    z3::expr right = std::visit(*this, *binary_expression.right);
    switch (binary_expression.type) {
    case ast::BinaryExpressionType::Plus:
      return left + right;
    case ast::BinaryExpressionType::Minus:
      return left - right;
    case ast::BinaryExpressionType::Star:
      return left * right;
    case ast::BinaryExpressionType::Slash:
      return left / right;
    case ast::BinaryExpressionType::And:
      return left && right;
    case ast::BinaryExpressionType::Or:
      return left || right;
    case ast::BinaryExpressionType::DoubleAnd:
      return z3::set_intersect(left, right);
    case ast::BinaryExpressionType::DoubleOr:
      return z3::set_union(left, right);
    case ast::BinaryExpressionType::BackSlash:
      return z3::set_difference(left, right);
    case ast::BinaryExpressionType::In:
      return z3::set_member(left, right);
    }
  }

  z3::expr operator()(const ast::UnaryExpression &unary_expression) {
    z3::expr expr = std::visit(*this, *unary_expression.expression);
    switch (unary_expression.type) {
    case ast::UnaryExpressionType::Minus:
      return -expr;
    case ast::UnaryExpressionType::Bang:
      return !expr;
    }
  }

  z3::expr operator()(const ast::TypeExpression & /* type_expression */) {
    LOG(FATAL) << "Tried to convert type expression to z3 expression, which is not supported";
  }

  z3::expr operator()(const ast::VarAccess &var_access) {
    DLOG(INFO) << "Creating z3 expr for VarAccess " << var_access;
    ast::VarAccess new_access {var_access.var_identifier, std::vector<ast::Identifier>()};
    auto it = var_to_expr.find(var_access.var_identifier.name);
    if (it == var_to_expr.end()) {
      DLOG(INFO) << "No symbol found for base \"" << new_access << "\", creating new one";
      const auto [it2, _] = var_to_expr.emplace(
          var_access.var_identifier.name,
          z3_stuff.context_.constant(
              var_access.var_identifier.name.GetString().c_str(),
              z3_stuff.sorts_.at(GetVarAccessType(new_access, ast, &context).identifier.name)));
      it = it2;
    }
    z3::expr expr_base = it->second;
    for (const auto &field : var_access.field_identifiers) {
      DLOG(INFO) << "Adding accessor " << field.name << "to current expr " << expr_base;
      const auto &type     = GetVarAccessType(new_access, ast, &context);
      const auto &accessor = z3_stuff.accessors_.at(type.identifier.name).at(field.name);
      expr_base            = accessor(expr_base);
      DLOG(INFO) << "Current expr is " << expr_base;
      new_access.field_identifiers.emplace_back(ast::Identifier {{parser::location()}, {field.name}});
    }
    return expr_base;
  }

  z3::expr operator()(const ast::ArrayAccess & /*value*/) {
    DLOG(FATAL) << "Unfinished function: "
                << "operator()(const ast::CollectionValue &value)";
  }

  z3::expr operator()(const ast::Value &value) {
    return std::visit(*this, value);
  }

  z3::expr operator()(const ast::ScalarValue<bool> &value) {
    return z3_stuff.context_.bool_val(value.value);
  }

  z3::expr operator()(const ast::ScalarValue<int64_t> &value) {
    return z3_stuff.context_.int_val(value.value);
  }

  z3::expr operator()(const ast::ScalarValue<uint64_t> &value) {
    return z3_stuff.context_.int_val(value.value);
  }

  z3::expr operator()(const ast::ScalarValue<double> &value) {
    return z3_stuff.context_.fpa_val(value.value);
  }

  z3::expr operator()(const ast::ScalarValue<std::string> &value) {
    return z3_stuff.context_.string_val(value.value);
  }

  z3::expr operator()(const ast::ConstructedValue &value) {
    z3::expr_vector args(z3_stuff.context_);
    for (const auto &[field_name, field] : value.fields) {
      args.push_back(std::visit(*this, *field));
    }
    return z3_stuff.constructors_.at(value.constructor_identifier.name)(args);
  }

  z3::expr operator()(const ast::CollectionValue & /*value*/) {
    DLOG(FATAL) << "Unfinished function: "
                << "operator()(const ast::CollectionValue &value)";
  }
};

[[nodiscard]] std::optional<Error> CompareExpressions(
    const ast::Expression &expected,
    const ast::Expression &got,
    Z3stuff &z3_stuff,
    const ast::AST &ast,
    std::deque<Scope *> &context);

} // namespace dbuf::checker
