#include <cassert>
#include <cctype>
#include <fstream>

#include "driver.hpp"

dbuf::Driver::~Driver() {
  delete (lexer);
  lexer = nullptr;
  delete (parser);
  parser = nullptr;
}

void dbuf::Driver::parse(const char *const filename) {
  /**
   * Remember, if you want to have checks in release mode
   * then this needs to be an if statement
   */
  assert(filename != nullptr);
  std::ifstream in_file(filename);
  if (!in_file.good()) {
    exit(EXIT_FAILURE);
  }
  parse_helper(in_file);
  return;
}

void dbuf::Driver::parse(std::istream &stream) {
  if (!stream.good() && stream.eof()) {
    return;
  }
  // else
  parse_helper(stream);
  return;
}

void dbuf::Driver::parse_helper(std::istream &stream) {

  delete (lexer);
  try {
    lexer = new dbuf::Lexer(&stream);
  } catch (std::bad_alloc &ba) {
    std::cerr << "Failed to allocate scanner: (" << ba.what()
              << "), exiting!!\n";
    exit(EXIT_FAILURE);
  }

  delete (parser);
  try {
    parser = new dbuf::Parser((*lexer) /* scanner */, (*this) /* driver */);
    parser->set_debug_level(1000);
  } catch (std::bad_alloc &ba) {
    std::cerr << "Failed to allocate parser: (" << ba.what()
              << "), exiting!!\n";
    exit(EXIT_FAILURE);
  }
  const int accept(0);
  if (parser->parse() != accept) {
    throw "Parse failed!\n";
  }
  return;
}
