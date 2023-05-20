/*
This file is part of DependoBuf project.

Copyright (C) 2023 Alexander Bogdanov, Alice Vernigor

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/
#pragma once

#if !defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif

#include "dbuf.tab.hpp"
#include "location.hh"

namespace dbuf::parser {

class Lexer : public yyFlexLexer {
public:
  explicit Lexer(std::istream &in, std::ostream &out)
      : yyFlexLexer(in, out) {};

  using FlexLexer::yylex;

  int yylex(Parser::semantic_type *lval, Parser::location_type *location);
};

} // namespace dbuf::parser
