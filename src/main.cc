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

  CLI::App app {"DependoBuf, type-safe serailization protocol"};

  std::string dbuf_file;
  std::string dir_path;
  std::vector<std::string> formats;
  app.add_option("-f,--file", dbuf_file, "dbuf file name")->required();
  app.add_option("-p,--path", dir_path, "path to generated files")->required();
  app.add_option("-o", formats, "required formats for generation")->required();

  CLI11_PARSE(app, argc, argv);
  return dbuf::Driver::Run(dbuf_file, dir_path, formats);
}
