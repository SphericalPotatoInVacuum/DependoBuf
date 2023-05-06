#include "core/checker/checker.h"

#include "core/ast/ast.h"
#include "core/checker/common.h"
#include "core/checker/name_resolution_checker.h"
#include "core/checker/positivity_checker.h"
#include "core/interning/interned_string.h"
#include "type_expression_checker.h"

#include <iostream>

namespace dbuf::checker {

ErrorList Checker::CheckNameResolution(const ast::AST &ast) {
  return NameResolutionChecker()(ast);
}

ErrorList Checker::CheckPositivity(const ast::AST &ast) {
  PositivityChecker::Result result = PositivityChecker()(ast);
  visit_order_                     = std::move(result.sorted);
  if (!result.errors.empty()) {
    return result.errors;
  }
  return {};
}

ErrorList
Checker::CheckTypeResolution(const ast::AST &ast, const std::vector<InternedString> &visit_order) {
  TypeExpressionChecker type_expression_checker(ast, visit_order);
  type_expression_checker.CheckTypes();
  return {};
}

int Checker::CheckAll(const ast::AST &ast) {
  ErrorList name_resolution_errors = CheckNameResolution(ast);
  if (!name_resolution_errors.empty()) {
    for (const auto &error : name_resolution_errors) {
      std::cerr << error.message << std::endl;
    }
    return EXIT_FAILURE;
  }

  ErrorList positivity_errors = CheckPositivity(ast);
  if (!positivity_errors.empty()) {
    for (const auto &error : positivity_errors) {
      std::cerr << error.message << std::endl;
    }
    return EXIT_FAILURE;
  }
  CheckTypeResolution(ast, visit_order_);

  return EXIT_SUCCESS;
}

} // namespace dbuf::checker
