#include "core/ast/ast.h"
#include "core/ast/expression.h"
#include "core/checker/positivity_checker.h"
#include "core/interning/interned_string.h"
#include "location.hh"

#include <exception>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <string>
#include <vector>

namespace dbuf {

class PositivityTest : public ::testing::TestSuite {
public:
  static ast::Message make_message(
      std::string name,
      const std::vector<std::pair<std::string, std::string>> &dependencies) {
    ast::Message message {
        {{parser::location(), InternedString(std::move(name))}},
    };
    for (const auto &[dep_name, dep_type] : dependencies) {
      message.type_dependencies.emplace_back(ast::TypedVariable {
          {InternedString(dep_name)},
          ast::TypeExpression {
              {parser::location()},
              {parser::location(), InternedString(dep_type)}}});
    }
    return message;
  }
};

/**
 * @brief Test that a dependency cycle between a message and itself is detected.
 *
 */
TEST(PositivityTest, SelfDependency) {
  ast::AST ast;

  ast::Message message_a = PositivityTest::make_message("A", {{"a", "A"}});

  ast.messages.insert(
      std::make_pair(InternedString(message_a.identifier.name), std::move(message_a)));

  checker::PositivityChecker::Result result = checker::PositivityChecker()(ast);

  ASSERT_EQ(result.errors.size(), 1);
  EXPECT_EQ(result.errors[0].message, "Found dependency cycle: A -> A");
}

/**
 * @brief Test that a dependency cycle between two messages is detected.
 *
 */
TEST(PositivityTest, CoDependency) {
  ast::AST ast;

  ast::Message message_a = PositivityTest::make_message("A", {{"b", "B"}});
  ast::Message message_b = PositivityTest::make_message("B", {{"a", "A"}});

  ast.messages.insert(
      std::make_pair(InternedString(message_a.identifier.name), std::move(message_a)));
  ast.messages.insert(
      std::make_pair(InternedString(message_b.identifier.name), std::move(message_b)));

  checker::PositivityChecker::Result result = checker::PositivityChecker()(ast);

  ASSERT_EQ(result.errors.size(), 1);
  EXPECT_EQ(result.errors[0].message, "Found dependency cycle: A -> B -> A");
}

/**
 * @brief Test that a no cycles found in the DAG.
 *
 */
TEST(PositivityTest, DAG) {
  ast::AST ast;

  ast::Message message_a = PositivityTest::make_message("A", {{"b", "B"}, {"c", "C"}});
  ast::Message message_b = PositivityTest::make_message("B", {{"c", "C"}, {"d", "D"}});

  ast::Message message_c = PositivityTest::make_message("C", {{"d", "D"}});
  ast::Message message_d = PositivityTest::make_message("D", {});

  ast.messages.insert(
      std::make_pair(InternedString(message_a.identifier.name), std::move(message_a)));
  ast.messages.insert(
      std::make_pair(InternedString(message_b.identifier.name), std::move(message_b)));
  ast.messages.insert(
      std::make_pair(InternedString(message_c.identifier.name), std::move(message_c)));
  ast.messages.insert(
      std::make_pair(InternedString(message_d.identifier.name), std::move(message_d)));

  checker::PositivityChecker::Result result = checker::PositivityChecker()(ast);

  ASSERT_EQ(result.errors.size(), 0);
}
}; // namespace dbuf
