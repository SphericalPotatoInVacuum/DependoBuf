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
#include "core/checker/expression_comparator.h"
#include "core/substitutor/substitutor.h"

#include <cstdint>
#include <optional>

namespace dbuf::checker {

class TypeComparator {
public:
  explicit TypeComparator(
      const ast::TypeExpression &expected,
      const ast::AST &ast,
      std::deque<Scope *> *context_ptr,
      Substitutor *substitutor_ptr,
      Z3stuff *z3_stuff_ptr);

  [[nodiscard]] std::optional<Error> Compare(const ast::Expression &expr);

  // Expression specifications
  std::optional<Error> operator()(const ast::TypeExpression & /* expr */);
  std::optional<Error> operator()(const ast::BinaryExpression &expr);
  std::optional<Error> operator()(const ast::UnaryExpression &expr);
  std::optional<Error> operator()(const ast::VarAccess &expr);
  std::optional<Error> operator()(const ast::ArrayAccess &expr);
  std::optional<Error> operator()(const ast::Value &val);

  // Value specifications
  template <typename T>
  std::optional<Error> operator()(const ast::ScalarValue<T> &val) {
    if (expected_.identifier.name != GetTypename(val)) {
      return Error(CreateError() << "Got value of type \"" << GetTypename(val) << "\", but expected type is \"" << expected_.identifier.name << "\" at " << val.location);
    }
    return {};
  }

  std::optional<Error> operator()(const ast::ConstructedValue &val);
  std::optional<Error> operator()(const ast::CollectionValue &val);

private:
  const ast::TypeExpression &expected_;
  const ast::AST &ast_;
  std::deque<Scope *> &context_;
  Substitutor &substitutor_;
  Z3stuff &z3_stuff_;

  ast::BinaryExpression bin_expr_;

  [[nodiscard]] std::optional<Error> CompareTypeExpressions(
      const ast::TypeExpression &expected_type,
      const ast::TypeExpression &expression,
      Z3stuff &z3_stuff);

  std::optional<Error> CheckConstructedValue(const ast::ConstructedValue &val, const ast::TypeWithFields &constructor);

  static InternedString GetTypename(const ast::ScalarValue<bool> &) {
    return InternedString("Bool");
  }

  static InternedString GetTypename(const ast::ScalarValue<int64_t> &) {
    return InternedString("Int");
  }

  static InternedString GetTypename(const ast::ScalarValue<uint64_t> &) {
    return InternedString("Unsigned");
  }

  static InternedString GetTypename(const ast::ScalarValue<double> &) {
    return InternedString("Float");
  }

  static InternedString GetTypename(const ast::ScalarValue<std::string> &) {
    return InternedString("String");
  }
};

}; // namespace dbuf::checker
