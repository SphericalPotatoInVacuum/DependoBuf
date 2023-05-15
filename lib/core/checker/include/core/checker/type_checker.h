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

  ErrorList CheckTypes();

  void operator()(const ast::Message &ast_message);
  void operator()(const ast::Enum &ast_enum);

private:
  /**
   * @brief Check that all dependencies are correctly defined
   *
   * @param type
   */
  void CheckDependencies(const ast::DependentType &type);
  /**
   * @brief Check that all fields are correctly defined
   *
   * @param type
   */
  void CheckFields(const ast::TypeWithFields &type);

  void CheckTypeExpression(const ast::TypeExpression &type_expression);

  const ast::AST &ast_;
  const std::vector<InternedString> sorted_graph_;

  Substitutor substitutor_;
  std::deque<Scope *> context_;
  ErrorList errors_;

  Z3stuff z3_stuff_;
};

} // namespace dbuf::checker
