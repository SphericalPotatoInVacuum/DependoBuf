#pragma once

#include <string>

namespace dbuf {

class Driver {
public:
  static int Run(const std::string &input_file, const std::string &output_file);
};

} // namespace dbuf
