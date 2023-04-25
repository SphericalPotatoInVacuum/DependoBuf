#include "core/checker/checker.h"

#include "core/ast/ast.h"
#include "core/checker/common.h"
#include "core/checker/positivity_checker.h"

#include <cstdlib>
#include <iostream>
#include <variant>

namespace dbuf::checker {

ErrorList Checker::CheckNameResolution(const ast::AST & /*ast*/) {
  // TODO (implement this)
  return {};
}

ErrorList Checker::CheckPositivity(const ast::AST &ast) {
  PositivityChecker::Result result = PositivityChecker()(ast);
  visit_order_                     = std::move(result.sorted);
  if (!result.errors.empty()) {
    return result.errors;
  }
  return {};
}

ErrorList Checker::CheckTypeResolution(const ast::AST & /*ast*/) {
  // TODO (implement this)
  return {};
}

int Checker::CheckAll(const ast::AST &ast) {
  CheckNameResolution(ast);
  ErrorList positivity_errors = CheckPositivity(ast);
  if (!positivity_errors.empty()) {
    for (const auto &error : positivity_errors) {
      std::cerr << error.message << std::endl;
    }
    return EXIT_FAILURE;
  }
  CheckTypeResolution(ast);

  return EXIT_SUCCESS;
}

} // namespace dbuf::checker
