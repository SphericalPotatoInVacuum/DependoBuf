/*
This file is part of DependoBuf project.

Copyright (C) 2023 Alexander Bogdanov, Alice Vernigor

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/
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

ErrorList Checker::CheckPositivity(ast::AST &ast) {
  PositivityChecker::Result result = PositivityChecker()(ast);
  ast.visit_order                  = std::move(result.sorted);
  if (!result.errors.empty()) {
    DLOG(INFO) << "Positivity errors: " << result.errors.size();
    return result.errors;
  }
  DLOG(INFO) << "Positivity check passed";
  std::stringstream ss;
  if (ast.visit_order.empty()) {
    return {};
  }
  ss << ast.visit_order[0];
  for (size_t i = 1; i < ast.visit_order.size(); ++i) {
    ss << " -> " << ast.visit_order[i];
  }
  DLOG(INFO) << "Visit order: " << ss.str();
  return {};
}

ErrorList Checker::CheckTypeResolution(const ast::AST &ast) {
  TypeChecker type_expression_checker(ast);
  return type_expression_checker.CheckTypes();
}

int Checker::CheckAll(ast::AST &ast) {
  DLOG(INFO) << "CheckAll AST\n" << ast::debug::ToString(ast);
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

  ErrorList type_errors = CheckTypeResolution(ast);
  if (!type_errors.empty()) {
    for (const auto &error : type_errors) {
      std::cerr << error.message << std::endl;
    }
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

} // namespace dbuf::checker
