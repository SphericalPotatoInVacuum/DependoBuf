#include "core/codegen/cpp_gen.h"
#include "core/driver/driver.h"

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <vector>

const std::string kSamplesPath        = "../../test/cpp_test/dbuf_files";
const std::string kCorrectSamplesPath = "../../test/cpp_test/correct_cpp_files";
const std::string kGenerationPath     = "./output";
std::vector<std::string> kFormat      = {"cpp"};

class CPPMessagesCorrectnessTest : public ::testing::TestWithParam<std::string> {
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

dbuf::Driver *CPPMessagesCorrectnessTest::driver_ = nullptr;

TEST_P(CPPMessagesCorrectnessTest, MessagesTest) {
  std::string filename      = GetParam();
  std::string dbuf_filename = filename + ".dbuf";
  std::string cpp_filename  = filename + ".h";

  ASSERT_EQ(driver_->Run(kSamplesPath + dbuf_filename, kGenerationPath, kFormat), EXIT_SUCCESS);

  std::ifstream generated(kGenerationPath + cpp_filename);
  std::ifstream required(kCorrectSamplesPath + cpp_filename);
  char actual, expexted;
  while (generated.get(actual) && required.get(expexted)) {
    ASSERT_EQ(generated.eofbit, required.eofbit);
    ASSERT_EQ(actual, expexted);
  }
}

INSTANTIATE_TEST_SUITE_P(
    CPPGenerationTest,
    CPPMessagesCorrectnessTest,
    testing::Values("/simple_messages", "/simple_enums", "/rt_dependent_messages", "/rt_dependent_enums"));