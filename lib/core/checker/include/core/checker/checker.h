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
#include "core/checker/common.h"
#include "core/interning/interned_string.h"

namespace dbuf::checker {

class Checker {
public:
  static int CheckAll(const ast::AST &ast);

private:
  static std::vector<InternedString> visit_order_;

  static std::optional<Error> CheckNameResolution(const ast::AST &ast);
  static std::optional<Error> CheckPositivity(const ast::AST &ast);
  static std::optional<Error> CheckTypeResolution(const ast::AST &ast, const std::vector<InternedString> &visit_order);
};

} // namespace dbuf::checker
