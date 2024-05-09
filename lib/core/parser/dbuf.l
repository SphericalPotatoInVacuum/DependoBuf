/*
This file is part of DependoBuf project.

Copyright (C) 2023 Alexander Bogdanov, Alice Vernigor

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/
%{
#include "core/parser/lexer.h"

#include <string>
#include <cstdlib>

#undef YY_DECL
#define YY_DECL int dbuf::parser::Lexer::yylex(dbuf::parser::Parser::semantic_type * lval, dbuf::parser::Parser::location_type *loc)

using token = dbuf::parser::Parser::token;

#define yyterminate() return token::TOK_END;
#define YY_USER_ACTION loc->step(); loc->columns(yyleng);
%}

%option debug
%option nodefault
%option yyclass="dbuf::parser::Lexer"
%option noyywrap
%option c++

lc_letter [a-z]
uc_letter [A-Z]
digit     [0-9]
ident_char [a-zA-Z0-9]
string_literal \"(\\.|[^\\"])*\"

%%

    /* Keywords */
"message" return token::TOK_MESSAGE;
"enum"    return token::TOK_ENUM;
"=>"      return token::TOK_IMPL;
"func"    return token::TOK_FUNC;
"->"      return token::TOK_ARROW;
"="       return token::TOK_EQUAL;
"false"   return token::TOK_FALSE;
"true"    return token::TOK_TRUE;

"in"  return token::TOK_IN;

    /* Literals */
{string_literal} {
  std::string str(yytext);
  // Remove the double quotes
  str = str.substr(1, str.size() - 2);
  lval->emplace<std::string>(str);
  return token::TOK_STRING_LITERAL;
}
{lc_letter}{ident_char}* {
  lval->emplace<std::string>(yytext);
  return token::TOK_LC_IDENTIFIER;
}
{uc_letter}{ident_char}* {
  lval->emplace<std::string>(yytext);
  return token::TOK_UC_IDENTIFIER;
}
{digit}+u {
  lval->emplace<uint64_t>(std::stoull(yytext));
  return token::TOK_UINT_LITERAL;
}
[+-]?{digit}+ {
  lval->emplace<int64_t>(std::stoll(yytext));
  return token::TOK_INT_LITERAL;
}
[+-]?{digit}+"."{digit}+ {
  lval->emplace<double>(std::stold(yytext));
  return token::TOK_FLOAT_LITERAL;
}

    /* One or two character tokens */
"&" return token::TOK_AND;
"|" return token::TOK_OR;
"!"  return token::TOK_BANG;
"&&"  return token::TOK_DOUBLE_AND;
"||"  return token::TOK_DOUBLE_OR;
"\\"  return token::TOK_BACK_SLASH;

    /* Single-character tokens */
"("  return token::TOK_LEFT_PAREN;
")"  return token::TOK_RIGHT_PAREN;
"{"  return token::TOK_LEFT_BRACE;
"}"  return token::TOK_RIGHT_BRACE;
"["  return token::TOK_LEFT_SQUARE;
"]"  return token::TOK_RIGHT_SQUARE;
"<"  return token::TOK_LEFT_TRIANGLE;
">"  return token::TOK_RIGHT_TRIANGLE;
","  return token::TOK_COMMA;
"."  return token::TOK_DOT;
"-"  return token::TOK_MINUS;
"+"  return token::TOK_PLUS;
"/"  return token::TOK_SLASH;
"*"  return token::TOK_STAR;
":"  return token::TOK_COLON;
";"  return token::TOK_SEMICOLON;

[ \t] /* spaces */
"\n" {
  loc->lines();
}

. {
  throw dbuf::parser::Parser::syntax_error(*loc, "invalid character: " + std::string(yytext));
}
<<EOF>> return token::TOK_END;

%%
