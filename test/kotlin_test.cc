#include "core/driver/driver.h"

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <string>
#include <vector>

const std::string kSamplePath           = "../../test/kotlin_test_samples/dbuf_files";
const std::string kExpectResultsPath    = "../../test/kotlin_test_samples/correct_kotlin_files";
const std::string kGenerationOutputPath = "./output";

class KotlinCodegenerationTest : public ::testing::TestWithParam<std::string> {
protected:
  static void SetUpTestSuit() {}
  static void TearDownTestSuit() {}

  void SetUp() override {
    driver_ = new dbuf::Driver();
    std::filesystem::create_directories(kGenerationOutputPath);
  }
  void TearDown() override {
    std::filesystem::remove_all(kGenerationOutputPath);
    delete driver_;
    driver_ = nullptr;
  }

  static dbuf::Driver *driver_;
};

dbuf::Driver *KotlinCodegenerationTest::driver_ = nullptr;

TEST_P(KotlinCodegenerationTest, CorrectGeneration) {
  std::string filename    = GetParam();
  std::string input_file  = kSamplePath + filename + ".dbuf";
  std::string expect_file = kExpectResultsPath + filename + ".kt";
  std::string output_file = kGenerationOutputPath + filename + ".kt";
  std::vector<std::string> output_formats {"kt"};

  ASSERT_EQ(driver_->Run(input_file, kGenerationOutputPath, output_formats), 0);

  std::ifstream generated(output_file);
  std::ifstream expect(expect_file);

  while (!generated.eof() && !expect.eof()) {
    ASSERT_EQ(generated.get(), expect.get());
  }
  ASSERT_EQ(generated.eof(), expect.eof());
}

INSTANTIATE_TEST_SUITE_P(KotlinTest, KotlinCodegenerationTest, testing::Values("/message", "/enum"));
