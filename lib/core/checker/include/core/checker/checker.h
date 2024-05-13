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
  using ErrorList = std::vector<Error>;

  static std::variant<ErrorList, ast::AST> ResolveGenerics(const ast::AST &ast);
  static ErrorList CheckNameResolution(const ast::AST &ast);
  static ErrorList CheckPositivity(ast::AST &ast);
  static ErrorList CheckTypeResolution(const ast::AST &ast);

  static int CheckAll(ast::AST &ast);
};

} // namespace dbuf::checker
