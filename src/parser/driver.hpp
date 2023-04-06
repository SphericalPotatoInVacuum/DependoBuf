#pragma once

#include "dbuf.tab.hpp"
#include "lexer.hpp"

#include <cstddef>
#include <istream>
#include <string>

namespace dbuf {
namespace parser {

class Driver {
public:
  Driver() = default;

  virtual ~Driver();

  /**
   * parse - parse from a file
   * @param filename - valid string with input file
   */
  void parse(const char *const filename);
  /**
   * parse - parse from a c++ input stream
   * @param is - std::istream&, valid input stream
   */
  void parse(std::istream &iss);

private:
  void parse_helper(std::istream &stream);

  Parser *parser = nullptr;
  Lexer *lexer   = nullptr;
};

} // namespace parser
} // namespace dbuf
