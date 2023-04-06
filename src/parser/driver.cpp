#include "driver.hpp"

#include <cassert>
#include <cctype>
#include <fstream>

namespace dbuf {
namespace parser {

Driver::~Driver() {
  delete (lexer);
  lexer = nullptr;
  delete (parser);
  parser = nullptr;
}

void Driver::parse(std::istream &stream) {
  if (!stream.good() && stream.eof()) {
    return;
  }
  // else
  parse_helper(stream);
  return;
}

void Driver::parse(const char *const filename) {
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

void Driver::parse_helper(std::istream &stream) {
  delete (lexer);
  try {
    lexer = new Lexer(&stream);
  } catch (std::bad_alloc &ba) {
    std::cerr << "Failed to allocate scanner: (" << ba.what() << "), exiting!!\n";
    exit(EXIT_FAILURE);
  }

  delete (parser);
  try {
    parser = new Parser((*lexer) /* scanner */, (*this) /* driver */);
    parser->set_debug_level(0);
  } catch (std::bad_alloc &ba) {
    std::cerr << "Failed to allocate parser: (" << ba.what() << "), exiting!!\n";
    exit(EXIT_FAILURE);
  }
  const int accept(0);
  if (parser->parse() != accept) {
    throw "Parse failed!\n";
  }
  return;
}

} // namespace parser
} // namespace dbuf
