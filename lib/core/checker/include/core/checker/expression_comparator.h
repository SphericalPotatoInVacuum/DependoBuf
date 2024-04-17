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

  using NameToSort        = std::unordered_map<InternedString, z3::sort>;        // NameToSort[type_name] = sort
  using NameToConstructor = std::unordered_map<InternedString, z3::func_decl>;   // NameToConstructor[cons_name] = cons
  using FieldToAccessor   = std::unordered_map<InternedString, z3::func_decl>;   // FieldToAccessor[field] = accessor
  using NameToFields      = std::unordered_map<InternedString, FieldToAccessor>; // NameToFields[type] = FieldToAccessor

  z3::context context_;
  z3::solver solver_;
  NameToSort sorts_;               // z3_sorts_[type_name] = sort
  NameToConstructor constructors_; // z3_constructors_[cons_name] = constructor
  NameToFields accessors_;         // z3_accessors_[cons_name][field_name] = accessor
};

struct ExpressionToZ3 {
  Z3stuff &z3_stuff;
  const ast::AST &ast;
  std::deque<Scope *> &context;
  std::unordered_map<InternedString, z3::expr> var_to_expr = {};

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
      return z3::set_member(right, left);
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

  z3::expr operator()(const ast::VarAccess &var_access) {
    DLOG(INFO) << "Creating z3 expr for VarAccess " << var_access;
    // Create a new empty var_access to gradually build a new one
    ast::VarAccess new_access {var_access.var_identifier, {}};
    // Try to find the symbol in the context
    auto it = var_to_expr.find(var_access.var_identifier.name);
    if (it == var_to_expr.end()) {
      DLOG(INFO) << "No symbol found for base \"" << new_access << "\", creating new one";
      // if not found, need to create one
      const auto [it2, _] = var_to_expr.emplace(
          var_access.var_identifier.name,
          z3_stuff.context_.constant(
              var_access.var_identifier.name.GetString().c_str(),
              z3_stuff.sorts_.at(GetVarAccessType(new_access, ast, &context).identifier.name)));
      it = it2;
    }
    z3::expr expr = it->second; // z3 symbol corresponding to the var_access base

    for (const auto &field : var_access.field_identifiers) {
      DLOG(INFO) << "Adding accessor " << field.name << "to current expr " << expr;
      // Try to find the accessor in the context
      const auto &type     = GetVarAccessType(new_access, ast, &context);
      const auto &accessor = z3_stuff.accessors_.at(type.identifier.name).at(field.name);
      expr                 = accessor(expr);
      DLOG(INFO) << "Current expr is " << expr;
      // Update the new_access
      new_access.field_identifiers.push_back(ast::Identifier {{parser::location()}, {field.name}});
    }
    return expr;
  } // NOLINT(clang-diagnostic-return-type)

  z3::expr operator()(const ast::ArrayAccess &/*value*/) {
    DLOG(FATAL) << "Unfinished function: " << "operator()(const ast::CollectionValue &value)";
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

  z3::expr operator()(const ast::Value &value) {
    return std::visit(*this, value);
  }

  z3::expr operator()(const ast::TypeExpression & /*type_expression*/) {
    LOG(FATAL) << "Tried to convert type expression to z3 expression, which is not supported";
  }

  z3::expr operator()(const ast::ConstructedValue &value) {
    z3::expr_vector args(z3_stuff.context_);
    for (const auto &[field_name, field] : value.fields) {
      args.push_back(std::visit(*this, *field));
    }
    return z3_stuff.constructors_.at(value.constructor_identifier.name)(args);
  }

  z3::expr operator()(const ast::CollectionValue &/*value*/) {
    DLOG(FATAL) << "Unfinished function: " << "operator()(const ast::CollectionValue &value)";
  }

  z3::expr operator()(const ast::Expression &expression) {
    return std::visit(*this, expression);
  }
};

[[nodiscard]] std::optional<Error> CompareExpressions(
    const ast::Expression &expected,
    const ast::Expression &got,
    Z3stuff &z3_stuff,
    const ast::AST &ast,
    std::deque<Scope *> &context);

} // namespace dbuf::checker
