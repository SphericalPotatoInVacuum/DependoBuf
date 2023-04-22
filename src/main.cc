#include "core/driver.h"

#include <cstdlib>
#include <cstring>
#include <iostream>

int main(const int argc, const char **argv) {
  if (argc == 3) {
    dbuf::Driver::Run(argv[1], argv[2]);
  } else {
    return (EXIT_FAILURE);
  }
  return (EXIT_SUCCESS);
}
