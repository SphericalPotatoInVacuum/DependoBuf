#include "core/driver/driver.h"

#include <exception>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <string>
#include <vector>

const std::string kSamplesPath              = "../../test/code_samples";
const std::string kCorrectSyntaxSamplesPath = kSamplesPath + "/type_check/";
const std::filesystem::directory_iterator kCorrectSyntaxSamplesIterator(kCorrectSyntaxSamplesPath);

class CompileTest : public ::testing::TestWithParam<std::filesystem::directory_entry> {
protected:
  static void SetUpTestSuite() {}

  static void TearDownTestSuite() {}

  void SetUp() override {
    driver_ = new dbuf::Driver();
  }

  void TearDown() override {
    delete driver_;
    driver_ = nullptr;
  }

  static dbuf::Driver *driver_;
};

dbuf::Driver *CompileTest::driver_ = nullptr;

TEST_P(CompileTest, WorksForCorrectSyntax) {
  ASSERT_EQ(driver_->Run(GetParam().path().string(), "logs"), 0) << "Parsing failed with exception";
}

INSTANTIATE_TEST_SUITE_P(
    CompileTestCorrectSyntax,
    CompileTest,
    ::testing::ValuesIn(begin(kCorrectSyntaxSamplesIterator), end(kCorrectSyntaxSamplesIterator)));
