#include "core/driver.h"

#include "core/ast/ast.h"
#include "core/checker/checker.h"
#include "core/parser/parse_helper.h"
#include "dbuf.tab.hpp"

#include <cassert>
#include <cctype>
#include <cstdlib>
#include <exception>
#include <fstream>

namespace dbuf {

int Driver::Run(const std::string &input_filename, const std::string &output_filename) {
  std::ifstream in_file(input_filename);
  if (!in_file.good()) {
    return EXIT_FAILURE;
  }

  std::ofstream out_file(output_filename);
  if (!out_file.good()) {
    return EXIT_FAILURE;
  }

  ast::AST ast;
  parser::ParseHelper parse_helper(in_file, std::cout, &ast);
  try {
    parse_helper.Parse();
  } catch (const parser::Parser::syntax_error &err) {
    std::cerr << "Uncaught syntax error: " << err.what() << std::endl;
    return EXIT_FAILURE;
  } catch (const char *err) {
    std::cerr << "Parsing error: " << err << std::endl;
    return EXIT_FAILURE;
  } catch (...) {
    std::cerr << "Something went wrong ¯\\_(ツ)_/¯" << std::endl;
    return EXIT_FAILURE;
  }
  if (parser::get_error_count() > 0) {
    std::cerr << "Compilation failed because of errors" << std::endl;
    return EXIT_FAILURE;
  }

  checker::Checker checker;
  return checker.CheckAll(ast);
}

} // namespace dbuf
