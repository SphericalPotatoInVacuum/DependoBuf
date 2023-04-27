#include "core/driver.h"

#include <cstdlib>
#include <cstring>
#include <iostream>

int main(const int argc, const char **argv) {
  if (argc == 3) {
    return dbuf::Driver::Run(argv[1], argv[2]);
  }
  return EXIT_FAILURE;
}
