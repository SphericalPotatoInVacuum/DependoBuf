#include <cstdlib>
#include <cstring>
#include <iostream>

#include <parser/driver.hpp>

int main(const int argc, const char **argv) {
  if (argc == 2) {
    dbuf::Driver driver;
    driver.parse(argv[1]);
  } else {
    return (EXIT_FAILURE);
  }
  return (EXIT_SUCCESS);
}
