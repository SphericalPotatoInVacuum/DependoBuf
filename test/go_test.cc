#include "core/codegen/go/go_gen.h"
#include "core/driver/driver.h"

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <vector>

const std::string kSamplesPath        = "../../test/go_test_samples/dbuf_files";
const std::string kCorrectSamplesPath = "../../test/go_test_samples/correct_go_files";
const std::string kGenerationPath     = "./output";

class GoGenerationCorrectnessTest : public ::testing::TestWithParam<std::string> {
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

dbuf::Driver *GoGenerationCorrectnessTest::driver_ = nullptr;

TEST_P(GoGenerationCorrectnessTest, MessagesTest) {
  std::string filename             = GetParam();
  std::string dbuf_filename        = filename + ".dbuf";
  std::string go_filename          = filename + ".go";
  std::vector<std::string> formats = {"go"};

  ASSERT_EQ(driver_->Run(kSamplesPath + dbuf_filename, kGenerationPath, formats), EXIT_SUCCESS);

  std::ifstream generated(kGenerationPath + go_filename);
  std::ifstream required(kCorrectSamplesPath + filename);
  char actual;
  char expexted;
  while (generated.get(actual) && required.get(expexted)) {
    ASSERT_EQ(generated.eofbit, required.eofbit);
    ASSERT_EQ(actual, expexted);
  }
}

INSTANTIATE_TEST_SUITE_P(
    GoGenerationTest,
    GoGenerationCorrectnessTest,
    testing::Values("/smoke", "/dependent_messages", "/dependent_enums"));
