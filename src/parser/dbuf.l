%{
#include <string>
#include <cstdlib>
#include <iostream>

#include <parser/lexer.hpp>

#undef YY_DECL
#define YY_DECL int dbuf::parser::Lexer::yylex(dbuf::parser::Parser::semantic_type *const lval, dbuf::parser::Parser::location_type *loc)

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

%%

%{/** Code executed at the beginning of yylex **/
  yylval = lval;
%}

    /* Keywords */
"message" {
  return token::TOK_MESSAGE;
}
"service" {return token::TOK_SERVICE;}
"false"   {return token::TOK_FALSE;}
"rpc"     {return token::TOK_RPC;}
"true"    {return token::TOK_TRUE;}
"enum"    {return token::TOK_ENUM;}
"=>"      {return token::TOK_IMPL;}
"returns" {return token::TOK_RETURNS;}

    /* Literals */
\"[^\r\n]*\" {
  yylval->build<std::string>(yytext);
  return token::TOK_STRING_LITERAL;
}
{lc_letter}{ident_char}* {
  yylval->build<std::string>(yytext);
  return token::TOK_LC_IDENTIFIER;
}
{uc_letter}{ident_char}* {
  yylval->build<std::string>(yytext);
  return token::TOK_UC_IDENTIFIER;
}
{digit}+ {
  yylval->build<long long>(atol(yytext));
  return token::TOK_INT_LITERAL;
}
{digit}+"."{digit}+ {
  yylval->build<double>(atof(yytext));
  return token::TOK_FLOAT_LITERAL;
}

    /* One or two character tokens */
"&&" return token::TOK_AND;
"||" return token::TOK_OR;
"!"  return token::TOK_BANG;

    /* Single-character tokens */
"("  {return token::TOK_LEFT_PAREN;}
")"  {return token::TOK_RIGHT_PAREN;}
"{"  {return token::TOK_LEFT_BRACE;}
"}"  {return token::TOK_RIGHT_BRACE;}
","  {return token::TOK_COMMA;}
"."  {return token::TOK_DOT;}
"-"  {return token::TOK_MINUS;}
"+"  {return token::TOK_PLUS;}
"/"  {return token::TOK_SLASH;}
"*"  {return token::TOK_STAR;}
":"  {return token::TOK_COLON;}

[ \t] /* spaces */
"\n" {
  loc->lines();
  return token::TOK_NL;
}

. printf("Unrecognized token: %s\n", yytext);

%%
