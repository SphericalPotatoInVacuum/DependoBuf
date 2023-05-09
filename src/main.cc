#include "core/driver.h"
#include "glog/logging.h"

#include <cstdlib>
#include <cstring>
#include <iostream>

int main(const int argc, const char **argv) {
  google::InitGoogleLogging(argv[0]);
  if (argc == 3) {
    return dbuf::Driver::Run(argv[1], argv[2]);
  }
  return EXIT_FAILURE;
}
