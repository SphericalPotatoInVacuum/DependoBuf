/*
This file is part of DependoBuf project.

Copyright (C) 2023 Alexander Bogdanov, Alice Vernigor

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/
#include "core/parser/lexer.h"
#include "dbuf.tab.hpp"

namespace dbuf::parser {

class ParseHelper {
public:
  ParseHelper(std::istream &in, std::ostream &out, ast::AST *ast)
      : lexer_(in, out)
      , parser_(&lexer_, ast) {};

  void Parse();

private:
  Lexer lexer_;
  DbufParser parser_;
};

} // namespace dbuf::parser
