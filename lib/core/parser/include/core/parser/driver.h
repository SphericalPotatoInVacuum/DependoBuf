#pragma once

#include "core/parser/lexer.h"
#include "dbuf.tab.hpp"

#include <cstddef>
#include <istream>
#include <string>

namespace dbuf::parser {

class Driver {
public:
  Driver() = default;

  virtual ~Driver();

  /**
   * parse - parse from a file
   * @param filename - valid string with input file
   */
  void parse(const char *filename);
  /**
   * parse - parse from a c++ input stream
   * @param is - std::istream&, valid input stream
   */
  void parse(std::istream &iss);

  void saveAst(AST);

private:
  void parse_helper(std::istream &stream);

  Parser *parser_ = nullptr;
  Lexer *lexer_   = nullptr;
  AST ast_;
};

} // namespace dbuf::parser
