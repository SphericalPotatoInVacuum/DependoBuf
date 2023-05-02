#pragma once

#include "core/ast/ast.h"
#include "core/ast/expression.h"
#include "core/checker/common.h"
#include "core/interning/interned_string.h"

#include <unordered_map>
#include <vector>

namespace dbuf::checker {

class PositivityChecker {
public:
  struct Result {
    std::vector<InternedString> sorted;
    ErrorList errors;
  };

  void operator()(const ast::Value & /* value */);

  void operator()(const ast::VarAccess & /* var_access */);

  void operator()(const ast::UnaryExpression &expr);

  void operator()(const ast::BinaryExpression &expr);

  void operator()(const ast::TypeExpression &type_expression);

  Result operator()(const ast::AST &ast);

private:
  enum class NodeState { Visiting, Visited };

  [[nodiscard]] Result TopSortGraph() const;

  std::vector<InternedString> Visit( // NOLINT(misc-no-recursion)
      const InternedString &name,
      std::vector<InternedString> &sorted,
      std::unordered_map<InternedString, NodeState> &node_states) const;

  DependencyGraph dependency_graph_;
  InternedString current_type_;
};

}; // namespace dbuf::checker
