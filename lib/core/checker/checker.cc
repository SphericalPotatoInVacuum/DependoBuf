#include "core/checker/checker.h"

#include "core/ast/ast.h"
#include "core/checker/common.h"
#include "core/checker/name_resolution_checker.h"
#include "core/checker/positivity_checker.h"
#include "core/checker/type_checker.h"
#include "core/interning/interned_string.h"
#include "glog/logging.h"

#include <iostream>
#include <sstream>

namespace dbuf::checker {

ErrorList Checker::CheckNameResolution(const ast::AST &ast) {
  return NameResolutionChecker()(ast);
}

ErrorList Checker::CheckPositivity(const ast::AST &ast) {
  PositivityChecker::Result result = PositivityChecker()(ast);
  visit_order_                     = std::move(result.sorted);
  if (!result.errors.empty()) {
    DLOG(INFO) << "Positivity errors: " << result.errors.size();
    return result.errors;
  }
  DLOG(INFO) << "Positivity check passed";
  std::stringstream ss;
  if (visit_order_.empty()) {
    return {};
  }
  ss << visit_order_[0];
  for (size_t i = 1; i < visit_order_.size(); ++i) {
    ss << " -> " << visit_order_[i];
  }
  DLOG(INFO) << "Visit order: " << ss.str();
  return {};
}

ErrorList Checker::CheckTypeResolution(const ast::AST &ast, const std::vector<InternedString> &visit_order) {
  TypeChecker type_expression_checker(ast, visit_order);
  return type_expression_checker.CheckTypes();
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

  ErrorList type_errors = CheckTypeResolution(ast, visit_order_);
  if (!type_errors.empty()) {
    for (const auto &error : type_errors) {
      std::cerr << error.message << std::endl;
    }
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

} // namespace dbuf::checker
