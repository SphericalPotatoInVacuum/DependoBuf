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
#include <optional>
#include <sstream>
#include <vector>

namespace dbuf::checker {

int Checker::CheckAll(const ast::AST &ast) {
  std::optional<Error> name_resolution_error = CheckNameResolution(ast);
  if (name_resolution_error.has_value()) {
    std::cerr << name_resolution_error->message << std::endl;
    return EXIT_FAILURE;
  }

  std::optional<Error> positivity_error = CheckPositivity(ast);
  if (positivity_error.has_value()) {
    std::cerr << positivity_error->message << std::endl;
    return EXIT_FAILURE;
  }

  std::optional<Error> type_error = CheckTypeResolution(ast, visit_order_);
  if (type_error.has_value()) {
    std::cerr << type_error->message << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

std::vector<InternedString> Checker::visit_order_ = {};

std::optional<Error> Checker::CheckNameResolution(const ast::AST &ast) {
  return NameResolutionChecker()(ast);
}

std::optional<Error> Checker::CheckPositivity(const ast::AST &ast) {
  PositivityChecker::Result result = PositivityChecker()(ast);
  if (result.error.has_value()) {
    return result.error;
  }

  DLOG(INFO) << "Positivity check passed";

  visit_order_ = std::move(result.sorted_order);
  if (visit_order_.empty()) {
    return {};
  }
  std::stringstream ss;
  ss << visit_order_[0];
  for (size_t i = 1; i < visit_order_.size(); ++i) {
    ss << " -> " << visit_order_[i];
  }

  DLOG(INFO) << "Visit order: " << ss.str();
  return {};
}

std::optional<Error> Checker::CheckTypeResolution(const ast::AST &ast, const std::vector<InternedString> &visit_order) {
  return TypeChecker(ast, visit_order).CheckTypes();
}

} // namespace dbuf::checker
