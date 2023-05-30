/*
This file is part of DependoBuf project.

Copyright (C) 2023 Alexander Bogdanov, Alice Vernigor

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/
#include "core/parser/lexer.h"
#include "dbuf.tab.hpp"
#include "glog/logging.h"

#include <gtest/gtest.h>
#include <sstream>
#include <vector>

namespace dbuf::test {

using token      = parser::Parser::token_kind_type;
using ParamTuple = std::tuple<std::string, std::vector<token>>;

class LexerTestSuite : public ::testing::TestWithParam<ParamTuple> {
protected:
  static void SetUpTestSuite() {}

  static void TearDownTestSuite() {}

  void SetUp() override {
    ss_    = new std::stringstream(std::get<0>(GetParam()));
    os_    = new std::ostringstream();
    lexer_ = new parser::Lexer(*ss_, *os_);
  }

  void TearDown() override {
    delete lexer_;
    lexer_ = nullptr;

    delete ss_;
    ss_ = nullptr;

    delete os_;
    os_ = nullptr;
  }

  static std::stringstream *ss_;
  static std::ostringstream *os_;
  static dbuf::parser::Lexer *lexer_;
};

parser::Lexer *LexerTestSuite::lexer_   = nullptr;
std::stringstream *LexerTestSuite::ss_  = nullptr;
std::ostringstream *LexerTestSuite::os_ = nullptr;

TEST_P(LexerTestSuite, LexerTest) {
  parser::Parser::semantic_type node;
  parser::Parser::location_type loc;
  std::vector<token> expected = std::get<1>(GetParam());
  std::vector<token> actual;
  try {
    for (int tok = lexer_->yylex(&node, &loc); tok != token::TOK_END; tok = lexer_->yylex(&node, &loc)) {
      actual.push_back(static_cast<token>(tok));
    }
  } catch (const std::exception &e) {
    LOG(ERROR) << e.what();
    LOG(ERROR) << os_->str();
  }
  ASSERT_EQ(actual, expected);
}

INSTANTIATE_TEST_SUITE_P(
    LexerTest,
    LexerTestSuite,
    testing::Values(
        ParamTuple(
            "message A (n B C{field: true})",
            {token::TOK_MESSAGE,
             token::TOK_UC_IDENTIFIER,
             token::TOK_LEFT_PAREN,
             token::TOK_LC_IDENTIFIER,
             token::TOK_UC_IDENTIFIER,
             token::TOK_UC_IDENTIFIER,
             token::TOK_LEFT_BRACE,
             token::TOK_LC_IDENTIFIER,
             token::TOK_COLON,
             token::TOK_TRUE,
             token::TOK_RIGHT_BRACE,
             token::TOK_RIGHT_PAREN}),
        ParamTuple("message", {token::TOK_MESSAGE}),
        ParamTuple("messages", {token::TOK_LC_IDENTIFIER})));

} // namespace dbuf::test
