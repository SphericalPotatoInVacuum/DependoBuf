/*
This file is part of DependoBuf project.

Copyright (C) 2023 Alexander Bogdanov, Alice Vernigor

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/
#include "core/driver/driver.h"

#include "core/ast/ast.h"
#include "core/checker/checker.h"
#include "core/codegen/generation.h"
#include "core/parser/parse_helper.h"
#include "dbuf.tab.hpp"

#include <cassert>
#include <cctype>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <vector>

namespace dbuf {

int Driver::Run(const std::string &input_filename, std::vector<const std::string> output_filenames) {
  std::ifstream in_file(input_filename);
  if (!in_file.good()) {
    return EXIT_FAILURE;
  }

  ast::AST ast;
  parser::ParseHelper parse_helper(in_file, std::cerr, &ast);
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

  checker::Checker checker;
  if (checker.CheckAll(ast) != EXIT_SUCCESS) {
    return EXIT_FAILURE;
  }

  gen::ListGenerators generators;
  try {
    generators.Fill(output_filenames);
  } catch (const char *err) {
    std::cerr << err << std::endl;
    return EXIT_FAILURE;
  }

  try {
    generators.Process(&ast);
  } catch (const char *err) {
    std::cerr << err << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

} // namespace dbuf
