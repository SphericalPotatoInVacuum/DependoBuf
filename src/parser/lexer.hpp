#pragma once

#include "dbuf.tab.hpp"

#if !defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif

#include "location.hh"

namespace dbuf {

class Lexer : public yyFlexLexer {
public:
  Lexer(std::istream *in) : yyFlexLexer(in) {
    loc = new dbuf::Parser::location_type();
  };

  // get rid of override virtual function warning
  using FlexLexer::yylex;

  virtual int yylex(dbuf::Parser::semantic_type *const lval,
                    dbuf::Parser::location_type *location);
  // YY_DECL defined in mc_lexer.l
  // Method body created by flex in mc_lexer.yy.cc

  char *getYytext() { return yytext; }

private:
  /* yyval ptr */
  dbuf::Parser::semantic_type *yylval = nullptr;
  /* location ptr */
  dbuf::Parser::location_type *loc = nullptr;
};

} // namespace dbuf
