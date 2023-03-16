#include <gtest/gtest.h>
#include <fstream>
#include <string>
#include <filesystem>
#include <vector>
#include "../src/parser/driver.hpp"


const std::string samples_path = "../test/code_samples";
const std::string correct_syntax_samples_path = samples_path + "/correct_syntax/";
const std::filesystem::directory_iterator correct_syntax_samples_iterator(correct_syntax_samples_path);

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
        std::string path = GetParam().path().u8string();
        input_file_.open(path);
        ASSERT_TRUE(input_file_.is_open()) << "Could not open input file: " << GetParam();
    }

    void TearDown() override {
        input_file_.close();
    }

    static dbuf::Driver* driver_;
    std::ifstream input_file_;
};
dbuf::Driver* ParserTest::driver_ = nullptr;

TEST_P(ParserTest, WorksForCorrectSyntax) {
    EXPECT_NO_THROW(driver_->parse(input_file_));
}

INSTANTIATE_TEST_SUITE_P(ParserTestCorrectSyntax, ParserTest,
                        ::testing::ValuesIn(
                            begin(correct_syntax_samples_iterator),
                            end(correct_syntax_samples_iterator)
                        ));        


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}