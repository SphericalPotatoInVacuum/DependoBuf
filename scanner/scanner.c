#include <stdio.h>

#include "lex/constants.h"

extern int yylex();
extern int yylineno;
extern char* yytext;

int main(void) {
    int token;

    token = yylex();
    while (token) {
        printf("%s\n", yytext);
    }

    return 0;
}
