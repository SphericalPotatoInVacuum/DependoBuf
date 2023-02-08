%{
#include "constants.h"
%}

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
  /*  \"[^\r\n]*\"      return  STRING;*/
    [a-z][a-zA-Z0-9]* return LC_IDENTIFIER;
    [A-Z][a-zA-Z0-9]* return UC_IDENTIFIER;
    [0-9]*            return NUMBER;
    [0-9]+"."[0-9]+   return FLOAT;

    /* One or two character tokens */
    "!=" return  BANG_EQUAL;
    ">=" return  GREATER_EQUAL;
    "<=" return  LESS_EQUAL;
    "&&" return  AND;
    "||" return  OR;
    "<"  return  LESS;
    "="  return  EQUAL;
    ">"  return  GREATER;
    "!"  return  BANG;

    /* Single-character tokens */
    "("  return  LEFT_PAREN;
    ")"  return  RIGHT_PAREN;
    "{"  return  LEFT_BRACE;
    "}"  return  RIGHT_BRACE;
    "["  return  LEFT_BRACKET;
    "]"  return  RIGHT_BRACKET;
    ","  return  COMMA;
    "."  return  DOT;
    "-"  return  MINUS;
    "+"  return  PLUS;
    "\\" return  SLASH;
    "*"  return  STAR;
%%

int yywrap(void) {
    return 1;
}
