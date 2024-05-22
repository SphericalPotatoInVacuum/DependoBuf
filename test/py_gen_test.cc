#include "core/codegen/python_target/py_gen.h"
#include "core/driver/driver.h"

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <vector>

const std::string kCodegenPath    = "../../test/code_samples/codegen";
const std::string kTestOutputPath = kCodegenPath + "/py_output";

class PyGenerationTest : public ::testing::TestWithParam<std::string> {
protected:
  static void SetUpTestSuite() {}

  static void TearDownTestSuite() {}

  void SetUp() override {
    driver_ = new dbuf::Driver();
    std::filesystem::create_directory(kTestOutputPath);
  }

  void TearDown() override {
    std::filesystem::remove_all(kTestOutputPath);
    delete driver_;
    driver_ = nullptr;
  }

  static dbuf::Driver *driver_;
};

dbuf::Driver *PyGenerationTest::driver_ = nullptr;

TEST_P(PyGenerationTest, CompareFiles) {
  std::string sample_name          = GetParam();
  std::string folder_name          = kCodegenPath + sample_name;
  std::string dbuf_filename        = sample_name + ".dbuf";
  std::string python_filename      = sample_name + ".py";
  std::vector<std::string> formats = {"py"};

  ASSERT_EQ(driver_->Run(folder_name + dbuf_filename, kTestOutputPath, formats), EXIT_SUCCESS);

  std::ifstream generated(kTestOutputPath + python_filename);
  std::ifstream required(folder_name + python_filename);
  char actual;
  char expexted;
  while (generated.get(actual) && required.get(expexted)) {
    ASSERT_EQ(generated.eofbit, required.eofbit);
    ASSERT_EQ(actual, expexted);
  }
}

INSTANTIATE_TEST_SUITE_P(
    PythonCharacterByCharacterCheck,
    PyGenerationTest,
    testing::Values(
        "/message_all_basic_types",
        "/enum_binary_tree",
        "/field_with_deps",
        "/expressions_as_deps",
        "/complex_enums"));