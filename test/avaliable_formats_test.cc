#include "core/codegen/generation.h"

#include <exception>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <string>
#include <vector>

using ParamTuple = std::tuple<bool, std::vector<std::string>>;

const std::string kDirPath  = "./generated_files";
const std::string kFileName = "test";

class AvaliableFormatsTest : public ::testing::TestWithParam<ParamTuple> {
protected:
  static void SetUpTestSuite() {}

  static void TearDownTestSuite() {}

  void SetUp() override {
    std::filesystem::create_directory(kDirPath);
    gens_ = new dbuf::gen::ListGenerators;
  }

  void TearDown() override {
    std::filesystem::remove_all(kDirPath);
    delete gens_;
    gens_ = nullptr;
  }

  static dbuf::gen::ListGenerators *gens_;
};

dbuf::gen::ListGenerators *AvaliableFormatsTest::gens_ = nullptr;

TEST_P(AvaliableFormatsTest, CorrectnessInputFormats) {
  bool expect_throw                = std::get<0>(GetParam());
  std::vector<std::string> formats = std::get<1>(GetParam());
  if (expect_throw) {
    ASSERT_THROW(gens_->Fill(formats, kDirPath, kFileName), const char *);
  } else {
    ASSERT_NO_THROW(gens_->Fill(formats, kDirPath, kFileName));
  }
}

INSTANTIATE_TEST_SUITE_P(
    ListGeneratorsFill,
    AvaliableFormatsTest,
    ::testing::Values(
        std::make_tuple(EXIT_FAILURE, std::vector<std::string> {"cpp", "c++", "py"}),
        std::make_tuple(EXIT_FAILURE, std::vector<std::string> {"c++", "cpp"}),
        std::make_tuple(EXIT_FAILURE, std::vector<std::string> {"txt", "c++", "py"}),
        std::make_tuple(EXIT_SUCCESS, std::vector<std::string> {"c++"}),
        std::make_tuple(EXIT_SUCCESS, std::vector<std::string> {"cpp"})));