%{
#include "constants.h"
%}

lc_letter [a-z]
uc_letter [A-Z]
letter    [a-zA-Z]
digit     [0-9]
ident_char [a-zA-Z0-9]

%%
    /* Keywords */
"message" return MESSAGE;
"service" return SERVICE;
"false"   return FALSE;
"rpc"     return RPC;
"print"   return PRINT;
"true"    return TRUE;
"enum"    return ENUM;
"=>"      return IMPL;
"return"  return RETURN;

    /* Literals */
\"[^\r\n]*\"             return STRING;
{lc_letter}{ident_char}* return LC_IDENTIFIER;
{uc_letter}{ident_char}* return UC_IDENTIFIER;
{digit}*                 return NUMBER;
{digit}+"."{digit}+      return FLOAT;

    /* One or two character tokens */
"!=" return BANG_EQUAL;
">=" return GREATER_EQUAL;
"<=" return LESS_EQUAL;
"&&" return AND;
"||" return OR;
"<"  return LESS;
"="  return EQUAL;
">"  return GREATER;
"!"  return BANG;

    /* Single-character tokens */
"("  return LEFT_PAREN;
")"  return RIGHT_PAREN;
"{"  return LEFT_BRACE;
"}"  return RIGHT_BRACE;
"["  return LEFT_BRACKET;
"]"  return RIGHT_BRACKET;
","  return COMMA;
"."  return DOT;
"-"  return MINUS;
"+"  return PLUS;
"/"  return SLASH;
"*"  return STAR;

[ \t] /* spaces */
\n   yylineno++;

.    printf("Unrecognized token: %s\n", yytext);

%%

int yywrap(void) {
    return 1;
}
