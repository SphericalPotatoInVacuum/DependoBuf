#include "core/driver/driver.h"
#include "glog/logging.h"

#include <cstdlib>
#include <cstring>
#include <iostream>

int main(const int argc, const char **argv) {
  google::InstallFailureSignalHandler();
  google::InitGoogleLogging(argv[0]);
  if (argc == 2) {
    return dbuf::Driver::Run(argv[1]);
  }
  return EXIT_FAILURE;
}
