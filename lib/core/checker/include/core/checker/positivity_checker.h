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

#include <map>
#include <vector>

namespace dbuf::checker {

class PositivityChecker {
public:
  struct Result {
    std::vector<InternedString> sorted;
    ErrorList errors;
  };

  void operator()(const ast::Star & /* star */);
  void operator()(const ast::Value & /* value */);

  void operator()(const ast::VarAccess & /* var_access */);
  void operator()(const ast::ArrayAccess & /* array_access */) {
    DLOG(FATAL) << "Unfinished function: " << "void operator()(const ast::ArrayAccess &array_access) ";
  }

  void operator()(const ast::UnaryExpression &expr);

  void operator()(const ast::BinaryExpression &expr);

  void operator()(const ast::TypeExpression &type_expression);

  void operator()(const ast::Enum &ast_enum);
  void operator()(const ast::Message &ast_message);

  Result operator()(const ast::AST &ast);

private:
  enum class NodeState { Visiting, Visited };

  [[nodiscard]] Result TopSortGraph() const;

  std::vector<InternedString> Visit( // NOLINT(misc-no-recursion)
      const InternedString &name,
      std::vector<InternedString> &sorted,
      std::map<InternedString, NodeState> &node_states) const;

  DependencyGraph dependency_graph_;
  InternedString current_type_;
  bool add_self_;
};

}; // namespace dbuf::checker
