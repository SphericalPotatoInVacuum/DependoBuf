#pragma once

#include "core/parser/lexer.h"
#include "dbuf.tab.hpp"

#include <cstddef>
#include <istream>
#include <string>
#include <unordered_map>

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

  uint64_t GetInterning(std::string &&input_string);

private:
  void parse_helper(std::istream &stream);

  Parser *parser_ = nullptr;
  Lexer *lexer    = nullptr;
  AST ast_;

  struct Interning {
    uint64_t GetInterning(std::string &&input_string);

    std::unordered_map<std::string, uint64_t> tokens_;
    uint64_t counter_ {0};
  };
  Interning interning_;
};

} // namespace dbuf::parser
