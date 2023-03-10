#include "scanner.h"

int main(void) {
  int token;
  int prevline;

  init(&token, &prevline);
  scan_input(&token, &prevline);
  
  return 0;
}
