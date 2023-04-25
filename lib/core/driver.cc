#include "core/driver.h"

#include "core/ast/ast.h"
#include "core/checker/checker.h"
#include "core/parser/parse_helper.h"

#include <cassert>
#include <cctype>
#include <fstream>

namespace dbuf {

int Driver::Run(const std::string &input_filename, const std::string &output_filename) {
  std::ifstream in_file(input_filename);
  if (!in_file.good()) {
    exit(EXIT_FAILURE);
  }

  std::ofstream out_file(output_filename);
  if (!out_file.good()) {
    exit(EXIT_FAILURE);
  }

  ast::AST ast;
  parser::ParseHelper parse_helper(in_file, std::cout, &ast);
  parse_helper.Parse();

  checker::Checker checker;
  return checker.CheckAll(ast);
}

} // namespace dbuf
