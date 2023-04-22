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
