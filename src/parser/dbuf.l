%{
#include "y.tab.h"
%}

lc_letter [a-z]
uc_letter [A-Z]
digit     [0-9]
ident_char [a-zA-Z0-9]

%%
    /* Keywords */
"message" return MESSAGE;
"service" return SERVICE;
"false"   return FALSE;
"rpc"     return RPC;
"true"    return TRUE;
"enum"    return ENUM;
"=>"      return IMPL;
"returns"  return RETURNS;

    /* Literals */
\"[^\r\n]*\"             return STRING_LITERAL;
{lc_letter}{ident_char}* return LC_IDENTIFIER;
{uc_letter}{ident_char}* return UC_IDENTIFIER;
{digit}+                 return INT_LITERAL;
{digit}+"."{digit}+      return FLOAT_LITERAL;

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
