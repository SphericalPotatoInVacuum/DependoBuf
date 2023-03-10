#include "scanner.h"
#include "y.tab.h"

#include <stdio.h>

extern int yylex();
extern int yylineno;
extern char *yytext;

void init(int* token, int* prevline) {
  *token = 0;
  *prevline = -1;
}

void scan_input(int* token, int* prevline) {
  while ((*token = yylex())) {
    if (yylineno != *prevline) {
      if (*prevline != -1) {
        printf("\n");
      }

      printf("%3d:", yylineno);
      *prevline = yylineno;
    }
    printf(" (%d", *token);

    if (*token >= LC_IDENTIFIER && *token <= FLOAT_LITERAL) {
      printf(": %s", yytext);
    }

    printf(")");
  }
  printf("\n");
}