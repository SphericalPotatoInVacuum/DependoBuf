/*
This file is part of DependoBuf project.

Copyright (C) 2023 Alexander Bogdanov, Alice Vernigor

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/
#pragma once

#include <string>
#include <vector>

namespace dbuf {

class Driver {
public:
  static int Run(const std::string &input_file, std::vector<const std::string> output_filenames = {});
};

} // namespace dbuf
