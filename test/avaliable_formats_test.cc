#include "core/driver/driver.h"

#include <exception>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <string>
#include <vector>

using ParamTuple = std::tuple<bool, std::vector<std::string>>;

const std::string kDirPath  = "./generated_files";
const std::string kFilePath = "../../test/code_samples/empty.dbuf";

class AvaliableFormatsTest : public ::testing::TestWithParam<ParamTuple> {
protected:
  static void SetUpTestSuite() {}

  static void TearDownTestSuite() {}

  void SetUp() override {
    std::filesystem::create_directory(kDirPath);
    driver_ = new dbuf::Driver();
  }

  void TearDown() override {
    std::filesystem::remove_all(kDirPath);
    delete driver_;
    driver_ = nullptr;
  }

  static dbuf::Driver *driver_;
};

dbuf::Driver *AvaliableFormatsTest::driver_ = nullptr;

TEST_P(AvaliableFormatsTest, CorrectnessInputFormats) {
  bool expected                    = std::get<0>(GetParam());
  std::vector<std::string> formats = std::get<1>(GetParam());
  ASSERT_EQ(driver_->Run(kFilePath, kDirPath, formats), expected);
}

INSTANTIATE_TEST_SUITE_P(
    DriverInput,
    AvaliableFormatsTest,
    ::testing::Values(
        std::make_tuple(EXIT_FAILURE, std::vector<std::string> {"cpp", "c++", "py"}),
        std::make_tuple(EXIT_FAILURE, std::vector<std::string> {"c++", "cpp"}),
        std::make_tuple(EXIT_FAILURE, std::vector<std::string> {"txt", "c++", "py"}),
        std::make_tuple(EXIT_SUCCESS, std::vector<std::string> {"c++"}),
        std::make_tuple(EXIT_SUCCESS, std::vector<std::string> {"cpp"})));