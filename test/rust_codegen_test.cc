/*
This file is part of DependoBuf project.

Copyright (C) 2023 Alexander Bogdanov, Alice Vernigor

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/
#include "core/ast/ast.h"
#include "core/codegen/rust_gen.h"
#include "core/parser/parse_helper.h"

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <ostream>
#include <string>

constexpr bool kGenerateMode = false;

const std::string kSamplesPath                = "../../test/code_samples/";
const std::string kRustSamplesPath            = kSamplesPath + "codegen/rust/";
const std::string kRustDbufsPath              = kRustSamplesPath + "samples/";
const std::string kCommonSamplesPath          = kSamplesPath + "correct_syntax/";
const std::string kCommonSamplesTypeCheckPath = kSamplesPath + "type_check/";
const std::string kCanonResultsPath           = kRustSamplesPath + "canon/";

using ParamTuple = std::tuple<std::string, std::string>;

class RustCodeGenTest : public ::testing::TestWithParam<ParamTuple> {
protected:
  static void SetUpTestSuite() {}

  static void TearDownTestSuite() {}

  void SetUp() override {
    std::string dbuf_file = get<0>(GetParam()) + get<1>(GetParam()) + ".dbuf";
    std::ifstream input_file;
    input_file.open(dbuf_file);
    ASSERT_TRUE(input_file.is_open()) << "Could not open input file: " << dbuf_file;

    ast_ = new dbuf::ast::AST();
    dbuf::parser::ParseHelper parse_helper(input_file, std::cout, ast_);
    parse_helper.Parse();
    input_file.close();
    out_file_ = kCanonResultsPath + get<1>(GetParam()) + "_generated.rs";
  }

  void TearDown() override {
    delete ast_;
    ast_ = nullptr;

    std::filesystem::path out_file_path(out_file_);
    if (!kGenerateMode) {
      std::filesystem::remove(out_file_path);
    }

    out_file_.clear();
  }
  std::string out_file_;
  dbuf::ast::AST *ast_;
};

bool FilesAreEqual(std::ifstream &lhs, std::ifstream &rhs) {
  char lchar;
  char rchar;
  while (!lhs.eof() && !rhs.eof()) {
    lchar = lhs.get();
    rchar = rhs.get();
    if (lchar != rchar) {
      return false;
    }
  }
  return lhs.eof() == rhs.eof();
}

TEST_P(RustCodeGenTest, GenerationIsOkay) {
  std::string rs_file = kCanonResultsPath + get<1>(GetParam()) + ".rs";

  for (const auto &[name, type_variant] : ast_->types) {
    ast_->visit_order.push_back(name);
  }

  {
    dbuf::gen::RustCodeGenerator generator(out_file_, true);
    generator.Generate(ast_);
  }

  std::ifstream canon_file(rs_file);
  std::ifstream generated_file(out_file_);
  ASSERT_TRUE(kGenerateMode || FilesAreEqual(canon_file, generated_file));
  canon_file.close();
  generated_file.close();
}

INSTANTIATE_TEST_SUITE_P(
    RustCodeGenTest,
    RustCodeGenTest,
    ::testing::Values(
        ParamTuple(kCommonSamplesPath, "complex_dependencies_arithmetic"),
        ParamTuple(kCommonSamplesPath, "complex_dependencies_bool"),
        ParamTuple(kCommonSamplesPath, "complex_dependencies"),
        ParamTuple(kCommonSamplesPath, "complex_fields_all_types"),
        ParamTuple(kCommonSamplesPath, "complex_fields_arithmetic"),
        ParamTuple(kCommonSamplesPath, "complex_fields_bool"),
        ParamTuple(kCommonSamplesPath, "constructros_with_fields"),
        ParamTuple(kCommonSamplesPath, "empty_message"),
        ParamTuple(kCommonSamplesPath, "fields_depend_on_dependencies_arithmetic"),
        ParamTuple(kCommonSamplesPath, "fields_depend_on_dependencies_bool"),
        ParamTuple(kCommonSamplesPath, "fields_depend_on_dependencies"),
        ParamTuple(kCommonSamplesPath, "fields_pattern_matching"),
        ParamTuple(kCommonSamplesPath, "literals"),
        ParamTuple(kCommonSamplesPath, "multiple_dependencies_pattern_matching"),
        ParamTuple(kCommonSamplesPath, "simple_constructors"),
        ParamTuple(kCommonSamplesPath, "simple_dependencies_arithmetic"),
        ParamTuple(kCommonSamplesPath, "simple_dependencies_fields"),
        ParamTuple(kCommonSamplesPath, "simple_dependencies"),
        ParamTuple(kCommonSamplesPath, "simple_fields"),
        ParamTuple(kCommonSamplesTypeCheckPath, "comparison"),
        ParamTuple(kCommonSamplesTypeCheckPath, "hard"),
        ParamTuple(kCommonSamplesTypeCheckPath, "pattern_match"),
        ParamTuple(kCommonSamplesTypeCheckPath, "simple")));
