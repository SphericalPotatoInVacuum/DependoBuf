#include "lex/constants.h"

#include <stdio.h>

extern int yylex();
extern int yylineno;
extern char *yytext;

int main(void) {
  int token;
  int prevline = -1;

  while ((token = yylex())) {
    if (yylineno != prevline) {
      if (prevline != -1)
        printf("\n");
      printf("%3d:", yylineno);
      prevline = yylineno;
    }
    printf(" (%d", token);
    if (token >= LC_IDENTIFIER && token <= FLOAT) {
      printf(": %s", yytext);
    }
    printf(")");
  }
  printf("\n");

  return 0;
}
