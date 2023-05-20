/*
This file is part of DependoBuf project.

Copyright (C) 2023 Alexander Bogdanov, Alice Vernigor

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/
#include "core/ast/ast.h"
#include "core/checker/positivity_checker.h"
#include "core/parser/parse_helper.h"

#include <cstring>
#include <exception>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <ios>
#include <string>
#include <string_view>
#include <vector>

namespace dbuf {

const std::string kSamplesPath = "../../test/code_samples/incorrect_syntax/";

using ParamTuple = std::tuple<std::string_view, std::string_view>;

class WellFoundednessTest : public ::testing::TestWithParam<ParamTuple> {
protected:
  static void SetUpTestSuite() {}

  static void TearDownTestSuite() {}

  void SetUp() override {
    input_file_.open(std::get<0>(GetParam()), std::ios_base::in);
    ASSERT_TRUE(input_file_.is_open()) << "Could not open input file: " << strerror(errno) << std::endl;

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
  static std::ifstream input_file_;
};

dbuf::parser::ParseHelper *WellFoundednessTest::parse_helper_ = nullptr;
dbuf::ast::AST *WellFoundednessTest::ast_                     = nullptr;
std::ifstream WellFoundednessTest::input_file_;

TEST_P(WellFoundednessTest, TypeDependencyCycleDetection) {
  ASSERT_NO_THROW(parse_helper_->Parse());

  checker::PositivityChecker::Result result = checker::PositivityChecker()(*ast_);

  if (std::get<1>(GetParam()).empty()) {
    ASSERT_EQ(result.errors.size(), 0) << std::get<1>(GetParam());
  } else {
    ASSERT_EQ(result.errors.size(), 1);
    EXPECT_EQ(result.errors[0].message, std::get<1>(GetParam()));
  }
}

INSTANTIATE_TEST_SUITE_P(
    TypeDependencyCycleDetection,
    WellFoundednessTest,
    testing::Values(
        ParamTuple("../../test/code_samples/incorrect_syntax/wf_message_self.dbuf", "Found dependency cycle: A -> A"),
        ParamTuple("../../test/code_samples/incorrect_syntax/wf_enum_self.dbuf", "Found dependency cycle: A -> A"),
        ParamTuple(
            "../../test/code_samples/incorrect_syntax/wf_message_other.dbuf",
            "Found dependency cycle: A -> B -> A"),
        ParamTuple(
            "../../test/code_samples/incorrect_syntax/wf_enum_other.dbuf",
            "Found dependency cycle: A -> B -> A"),
        ParamTuple(
            "../../test/code_samples/incorrect_syntax/wf_mixed.dbuf",
            "Found dependency cycle: A -> C -> B -> A"),
        ParamTuple("../../test/code_samples/incorrect_syntax/wf_enum_field_self.dbuf", ""),
        ParamTuple(
            "../../test/code_samples/incorrect_syntax/wf_mixed_field.dbuf",
            "Found dependency cycle: A -> B -> C -> A")));

} // namespace dbuf
