%{
#include <string>
#include <cstdlib>
#include <iostream>

#include <parser/lexer.hpp>

#undef YY_DECL
#define YY_DECL int dbuf::Lexer::yylex(dbuf::Parser::semantic_type *const lval, dbuf::Parser::location_type *loc)

using token = dbuf::Parser::token;

#define yyterminate() return token::END;
#define YY_USER_ACTION loc->step(); loc->columns(yyleng);
%}

%option debug
%option nodefault
%option yyclass="dbuf::Lexer"
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
  return token::MESSAGE;
}
"service" {return token::SERVICE;}
"false"   {return token::FALSE;}
"rpc"     {return token::RPC;}
"true"    {return token::TRUE;}
"enum"    {return token::ENUM;}
"=>"      {return token::IMPL;}
"returns" {return token::RETURNS;}

    /* Literals */
\"[^\r\n]*\" {
  yylval->build<std::string>(yytext);
  return token::STRING_LITERAL;
}
{lc_letter}{ident_char}* {
  yylval->build<std::string>(yytext);
  return token::LC_IDENTIFIER;
}
{uc_letter}{ident_char}* {
  yylval->build<std::string>(yytext);
  return token::UC_IDENTIFIER;
}
{digit}+ {
  yylval->build<long long>(atol(yytext));
  return token::INT_LITERAL;
}
{digit}+"."{digit}+ {
  yylval->build<double>(atof(yytext));
  return token::FLOAT_LITERAL;
}

    /* One or two character tokens */
"!=" return token::BANG_EQUAL;
">=" return token::GREATER_EQUAL;
"<=" return token::LESS_EQUAL;
"&&" return token::AND;
"||" return token::OR;
"<"  return token::LESS;
"="  return token::EQUAL;
">"  return token::GREATER;
"!"  return token::BANG;

    /* Single-character tokens */
"("  {return token::LEFT_PAREN;}
")"  {return token::RIGHT_PAREN;}
"{"  {return token::LEFT_BRACE;}
"}"  {return token::RIGHT_BRACE;}
","  {return token::COMMA;}
"."  {return token::DOT;}
"-"  {return token::MINUS;}
"+"  {return token::PLUS;}
"/"  {return token::SLASH;}
"*"  {return token::STAR;}
":"  {return token::COLON;}

[ \t] /* spaces */
"\n" {
  loc->lines();
  return token::NL;
}

. printf("Unrecognized token: %s\n", yytext);

%%
