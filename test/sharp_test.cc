#include "core/codegen/sharp_gen.h"
#include "core/driver/driver.h"

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <vector>

const std::string kSamplesPath        = "../../test/sharp_test_samples/dbuf_files";
const std::string kCorrectSamplesPath = "../../test/sharp_test_samples/correct_sharp_files";
const std::string kGenerationPath     = "./output";

class SharpCodeGenCorrectnessTest : public ::testing::TestWithParam<std::string> {
protected:
  static void SetUpTestSuite() {}

  static void TearDownTestSuite() {}

  void SetUp() override {
    driver_ = new dbuf::Driver();
    std::filesystem::create_directory(kGenerationPath);
  }

  void TearDown() override {
    std::filesystem::remove_all(kGenerationPath);
    delete driver_;
    driver_ = nullptr;
  }

  static dbuf::Driver *driver_;
};

dbuf::Driver *SharpCodeGenCorrectnessTest::driver_ = nullptr;

TEST_P(SharpCodeGenCorrectnessTest, MessagesTest) {
  std::string filename             = GetParam();
  std::string dbuf_filename        = filename + ".dbuf";
  std::string cpp_filename         = filename + ".cs";
  std::vector<std::string> formats = {"cs"};

  ASSERT_EQ(driver_->Run(kSamplesPath + dbuf_filename, kGenerationPath, formats), EXIT_SUCCESS);

  std::ifstream generated(kGenerationPath + cpp_filename);
  std::ifstream required(kCorrectSamplesPath + filename);
  char actual;
  char expexted;
  while (generated.get(actual) && required.get(expexted)) {
    ASSERT_EQ(generated.eofbit, required.eofbit);
    ASSERT_EQ(actual, expexted);
  }
}

INSTANTIATE_TEST_SUITE_P(
    GenerationTest,
    SharpCodeGenCorrectnessTest,
    testing::Values("/simple_message", "/simple_enum", "/dependent_message", "/dependent_enum"));