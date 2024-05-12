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

int Driver::Run(const std::string &input_filename, const std::string &path, std::vector<std::string> &output_formats) {
  std::ifstream in_file(input_filename);
  if (!in_file.good()) {
    std::cerr << "File is bad: " << input_filename << std::endl;
    return EXIT_FAILURE;
  }
  auto name_start            = input_filename.find_last_of('/');
  auto name_end              = input_filename.find_last_of('.');
  name_start                 = (name_start == std::string::npos) ? -1 : name_start;
  const std::string filename = input_filename.substr(name_start + 1, name_end - name_start - 1);

  gen::ListGenerators generators;
  try {
    generators.Fill(output_formats, path, filename);
  } catch (std::string &err) {
    std::cerr << err << std::endl;
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

  if (dbuf::checker::Checker::CheckAll(ast) != EXIT_SUCCESS) {
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
