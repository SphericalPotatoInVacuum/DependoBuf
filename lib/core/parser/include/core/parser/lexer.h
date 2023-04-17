#pragma once

#if !defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif

#include "dbuf.tab.hpp"
#include "location.hh"

namespace dbuf {
namespace parser {

class Lexer : public yyFlexLexer {
public:
  Lexer(std::istream *in) : yyFlexLexer(in) { loc = new Parser::location_type(); };

  // get rid of override virtual function warning
  using FlexLexer::yylex;

  virtual int yylex(Parser::semantic_type *const lval, Parser::location_type *location);
  // YY_DECL defined in mc_lexer.l
  // Method body created by flex in mc_lexer.yy.cc

private:
  /* yyval ptr */
  Parser::semantic_type *yylval = nullptr;
  /* location ptr */
  Parser::location_type *loc = nullptr;
};

} // namespace parser
} // namespace dbuf
