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
#include "core/interning/interned_string.h"
#include "glog/logging.h"

#include <deque>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <variant>
#include <vector>

namespace dbuf {

struct Substitutor {
  void AddSubstitution(InternedString name, const std::shared_ptr<const ast::Expression> &expression);
  void PushScope();
  void PopScope();

  template <typename T>
  ast::Expression operator()(const ast::ScalarValue<T> &value) {
    return ast::ScalarValue<T>(value);
  }

  ast::Expression operator()(const ast::BinaryExpression &expression);
  ast::Expression operator()(const ast::UnaryExpression &expression);

  ast::Expression operator()(const ast::VarAccess &value, const ast::ConstructedValue &substitution);
  ast::Expression operator()(const ast::VarAccess &value, const ast::VarAccess &substitution);
  template <typename T>
  ast::Expression operator()(const ast::VarAccess &value, const ast::ScalarValue<T> &substitution) {
    DCHECK(value.field_identifiers.empty()) << "Field access on scalar value";
    return substitution;
  }
  ast::Expression operator()(const ast::VarAccess &value, const ast::Value &substitution) {
    return std::visit(*this, ast::Expression(value), substitution);
  }
  template <typename T, typename U>
  ast::Expression operator()(const T &, const U &) {
    throw std::runtime_error("Substitution error");
  }

  ast::Expression operator()(const ast::VarAccess &value);
  ast::Expression operator()(const ast::ArrayAccess &/*value*/) {
    DLOG(FATAL) << "Unfinished function: " << "ast::Expression operator()(const ast::ArrayAccess &value)";
  }

  ast::Expression operator()(const ast::ConstructedValue &value);
  ast::Expression operator()(const ast::CollectionValue &/*value*/) {
    DLOG(FATAL) << "Unfinished function: " << "ast::Expression operator()(const ast::CollectionValue &value)";
  }

  ast::Expression operator()(const ast::Value &value);

  ast::Expression operator()(const ast::TypeExpression &type_expression);

private:
  using Scope = std::unordered_map<InternedString, std::shared_ptr<const ast::Expression>>;
  std::deque<Scope> substitute_;
};

} // namespace dbuf
