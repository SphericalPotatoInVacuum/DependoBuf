#pragma once

#include <string>

namespace dbuf {

class Driver {
public:
  static int Run(const std::string &input_file);
};

} // namespace dbuf
