/*
This file is part of DependoBuf project.

Copyright (C) 2023 Alexander Bogdanov, Alice Vernigor

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/
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
