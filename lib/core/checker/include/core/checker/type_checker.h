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
#include "core/interning/interned_string.h"
#include "core/substitutor/substitutor.h"
#include "glog/logging.h"
#include "location.hh"
#include "z3++.h"

#include <deque>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace dbuf::checker {

class TypeChecker {
public:
  explicit TypeChecker(const ast::AST &ast, const std::vector<InternedString> &sorted_graph);

  [[nodiscard]] std::optional<Error> CheckTypes();

  bool operator()(const ast::Message &ast_message);
  bool operator()(const ast::Enum &ast_enum);

private:
  const ast::AST &ast_;
  const std::vector<InternedString> &sorted_graph_;

  std::deque<Scope *> context_;
  Substitutor substitutor_;
  Z3stuff z3_stuff_;
  std::optional<Error> error_;

  bool CheckDependencies(const ast::DependentType &type);
  bool CheckFields(const ast::TypeWithFields &type);

  bool CheckTypeExpression(const ast::TypeExpression &type_expression);
};

} // namespace dbuf::checker
