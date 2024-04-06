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
#include <set>

int main(const int argc, const char **argv) {
  google::InstallFailureSignalHandler();
  google::InitGoogleLogging(argv[0]);
  if (argc == 2) {
    return dbuf::Driver::Run(argv[1]);
  }

  if (argc >= 4) {
    if (std::strcmp(argv[2], "-o") != 0) {
      return EXIT_FAILURE;
    }

    std::vector<std::string> filenames;
    filenames.reserve(argc - 3);

    for (int file_no = 3; file_no < argc; ++file_no) {
      filenames.emplace_back(argv[file_no]);
    }
    
    return dbuf::Driver::Run(argv[1], filenames);
  }
  return EXIT_FAILURE;
}
