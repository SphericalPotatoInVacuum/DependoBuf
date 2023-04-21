#include "core/driver.h"

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
  static void SetUpTestSuite() {
    if (driver_ == nullptr) {
      driver_ = new dbuf::Driver;
    }
  }

  static void TearDownTestSuite() {
    delete driver_;
    driver_ = nullptr;
  }

  void SetUp() override {
    std::string path = GetParam().path().string();
    input_file_.open(path);
    ASSERT_TRUE(input_file_.is_open()) << "Could not open input file: " << GetParam();
  }

  void TearDown() override {
    input_file_.close();
  }

  static dbuf::Driver *driver_;
  std::ifstream input_file_;
};
dbuf::Driver *ParserTest::driver_ = nullptr;

TEST_P(ParserTest, WorksForCorrectSyntax) {
  try {
    driver_->parse(input_file_);
  } catch (std::exception &e) { FAIL() << "Parsing failed with exception: " << e.what(); }
}

INSTANTIATE_TEST_SUITE_P(
    ParserTestCorrectSyntax,
    ParserTest,
    ::testing::ValuesIn(begin(kCorrectSyntaxSamplesIterator), end(kCorrectSyntaxSamplesIterator)));

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
