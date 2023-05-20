/*
This file is part of DependoBuf project.

Copyright (C) 2023 Alexander Bogdanov, Alice Vernigor

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/
#include "core/ast/ast.h"
#include "core/parser/parse_helper.h"

#include <exception>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <string>
#include <vector>

const std::string kSamplesPath              = "../../test/code_samples";
const std::string kCorrectSyntaxSamplesPath = kSamplesPath + "/correct_syntax/";
const std::filesystem::directory_iterator kCorrectSyntaxSamplesIterator(kCorrectSyntaxSamplesPath);

class ParserTest : public ::testing::TestWithParam<std::filesystem::directory_entry> {
protected:
  static void SetUpTestSuite() {}

  static void TearDownTestSuite() {}

  void SetUp() override {
    std::string path = GetParam().path().string();
    input_file_.open(path);
    ASSERT_TRUE(input_file_.is_open()) << "Could not open input file: " << GetParam();

    ast_          = new dbuf::ast::AST();
    parse_helper_ = new dbuf::parser::ParseHelper(input_file_, std::cout, ast_);
  }

  void TearDown() override {
    delete parse_helper_;
    parse_helper_ = nullptr;

    delete ast_;
    ast_ = nullptr;

    input_file_.close();
  }

  static dbuf::parser::ParseHelper *parse_helper_;
  static dbuf::ast::AST *ast_;
  std::ifstream input_file_;
};

dbuf::parser::ParseHelper *ParserTest::parse_helper_ = nullptr;
dbuf::ast::AST *ParserTest::ast_                     = nullptr;

TEST_P(ParserTest, WorksForCorrectSyntax) {
  ASSERT_NO_THROW(parse_helper_->Parse()) << "Parsing failed with exception";
}

INSTANTIATE_TEST_SUITE_P(
    ParserTestCorrectSyntax,
    ParserTest,
    ::testing::ValuesIn(begin(kCorrectSyntaxSamplesIterator), end(kCorrectSyntaxSamplesIterator)));
