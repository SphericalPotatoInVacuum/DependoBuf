#include "core/parser/driver.h"

#include <cassert>
#include <cctype>
#include <fstream>

namespace dbuf::parser {

Driver::~Driver() {
  delete (lexer_);
  lexer_ = nullptr;
  delete (parser_);
  parser_ = nullptr;
}

void Driver::parse(std::istream &iss) {
  if (!iss.good() && iss.eof()) {
    return;
  }
  // else
  parse_helper(iss);
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
}

void Driver::saveAst(AST ast) {
  ast_ = std::move(ast);
}

void Driver::parse_helper(std::istream &stream) {
  delete (lexer_);
  try {
    lexer_ = new Lexer(&stream);
  } catch (std::bad_alloc &ba) {
    std::cerr << "Failed to allocate scanner: (" << ba.what() << "), exiting!!\n";
    exit(EXIT_FAILURE);
  }

  delete (parser_);
  try {
    parser_ = new Parser((*lexer_) /* scanner */, (*this) /* driver */);
    parser_->set_debug_level(0);
  } catch (std::bad_alloc &ba) {
    std::cerr << "Failed to allocate parser: (" << ba.what() << "), exiting!!\n";
    exit(EXIT_FAILURE);
  }
  const int accept(0);
  if (parser_->parse() != accept) {
    throw "Parse failed!\n";
  }
}

} // namespace dbuf::parser