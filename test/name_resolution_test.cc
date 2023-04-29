#include "core/ast/ast.h"
#include "core/ast/expression.h"
#include "core/checker/common.h"
#include "core/checker/name_resolution_checker.h"
#include "core/interning/interned_string.h"

#include <gtest/gtest.h>
#include <memory>
#include <unordered_set>
#include <utility>
#include <vector>

namespace dbuf {

class NameResolutionTest : public ::testing::TestSuite {
public:
  static ast::TypedVariable
  make_typed_variable(InternedString &&name, ast::TypeExpression &&type_expression) {
    return ast::TypedVariable {
        .name            = std::move(name),
        .type_expression = std::move(type_expression),
    };
  }

  template <typename T>
  static std::pair<InternedString, std::unique_ptr<ast::Expression>>
  make_field_assigment(std::string &&field_name, T value) {
    std::unique_ptr<ast::Expression> ptr =
        std::make_unique<ast::Expression>(ast::Value(ast::ScalarValue<T> {.value = value}));
    return std::make_pair(
        InternedString(field_name),
        std::make_unique<ast::Expression>(ast::ScalarValue<T> {.value = value}));
  }

  static ast::ConstructedValue make_constructed_value(
      std::string &&constructor_identifier,
      std::vector<std::pair<InternedString, std::unique_ptr<ast::Expression>>> &&fields) {
    return ast::ConstructedValue {
        .constructor_identifier = InternedString(std::move(constructor_identifier)),
        .fields                 = std::move(fields)};
  }

  static ast::TypedVariable make_simple_typed_variable(std::string &&name, std::string &&type) {
    return ast::TypedVariable {
        .name            = InternedString(std::move(name)),
        .type_expression = ast::TypeExpression {.name = InternedString(std::move(type))},
    };
  }

  static ast::TypeExpression make_type_expression(
      InternedString &&name,
      std::vector<std::unique_ptr<ast::Expression>> &&parameters) {
    return ast::TypeExpression {.name = std::move(name), .parameters = std::move(parameters)};
  }

  static ast::Message make_message(
      std::string &&name,
      std::vector<ast::TypedVariable> &&type_dependencies,
      std::vector<ast::TypedVariable> &&fields) {
    return ast::Message {
        .name              = InternedString(std::move(name)),
        .type_dependencies = std::move(type_dependencies),
        .fields            = std::move(fields)};
  }

  static ast::Constructor
  make_constructor(std::string &&name, std::vector<ast::TypedVariable> &&fields) {
    return ast::Constructor {.name = InternedString(std::move(name)), .fields = std::move(fields)};
  }

  static ast::Enum make_enum(
      std::string &&name,
      std::vector<ast::TypedVariable> &&type_dependencies,
      std::vector<ast::Enum::Rule> &&pattern_mapping) {
    return ast::Enum {
        .name              = InternedString(std::move(name)),
        .type_dependencies = std::move(type_dependencies),
        .pattern_mapping   = std::move(pattern_mapping)};
  }

  static ast::Enum::Rule make_rule(
      std::vector<ast::Enum::Rule::InputPattern> &&inputs,
      std::vector<ast::Constructor> &&outputs) {
    return ast::Enum::Rule {.inputs = std::move(inputs), .outputs = std::move(outputs)};
  }
};

TEST(NameResolutionTest, GlobalNamesRepetition) {
  ast::AST ast;

  ast::Message message_a            = NameResolutionTest::make_message("A", {}, {});
  ast.messages[InternedString("A")] = std::move(message_a);

  ast::Constructor constructor_a = NameResolutionTest::make_constructor("A", {});
  std::vector<ast::Constructor> outputs;
  outputs.emplace_back(std::move(constructor_a));

  std::vector<ast::Enum::Rule> patterns;
  patterns.emplace_back(NameResolutionTest::make_rule({}, std::move(outputs)));

  ast::Enum enum_a               = NameResolutionTest::make_enum("A", {}, std::move(patterns));
  ast.enums[InternedString("A")] = std::move(enum_a);

  std::unordered_set<std::string> expected_errors {
      "Re-declaration of enum: \"A\"",
      "Re-declaration of constructor: \"A\""};

  checker::ErrorList errors = checker::NameResolutionChecker()(ast);

  ASSERT_EQ(errors.size(), 2);

  for (auto &error : errors) {
    EXPECT_TRUE(expected_errors.contains(error.message));
  }
}

TEST(NameResolutionTest, UnknownTypename) {
  ast::AST ast;

  std::vector<ast::TypedVariable> dependencies;
  dependencies.emplace_back(NameResolutionTest::make_simple_typed_variable("a", "B"));

  std::vector<ast::TypedVariable> fields;
  fields.emplace_back(NameResolutionTest::make_simple_typed_variable("field1", "C"));

  ast::Message message_a =
      NameResolutionTest::make_message("A", std::move(dependencies), std::move(fields));
  ast.messages[InternedString("A")] = std::move(message_a);

  std::unordered_set<std::string> expected_errors {
      "Undefined type name: \"B\"",
      "Undefined type name: \"C\""};

  checker::ErrorList errors = checker::NameResolutionChecker()(ast);

  ASSERT_EQ(errors.size(), 2);

  for (auto &error : errors) {
    EXPECT_TRUE(expected_errors.contains(error.message));
  }
}

TEST(NameResolutionTest, FieldRedeclaration) {
  ast::AST ast;

  std::vector<ast::TypedVariable> message_fields;
  message_fields.emplace_back(NameResolutionTest::make_simple_typed_variable("field1", "Int"));
  message_fields.emplace_back(NameResolutionTest::make_simple_typed_variable("field1", "Bool"));
  message_fields.emplace_back(NameResolutionTest::make_simple_typed_variable("field1", "Int"));
  ast::Message message_a = NameResolutionTest::make_message("A", {}, std::move(message_fields));
  ast.messages[InternedString("A")] = std::move(message_a);

  std::vector<ast::TypedVariable> constructor_fields;
  constructor_fields.emplace_back(NameResolutionTest::make_simple_typed_variable("field2", "Int"));
  constructor_fields.emplace_back(NameResolutionTest::make_simple_typed_variable("field2", "Bool"));

  std::vector<ast::Constructor> outputs;
  outputs.emplace_back(
      NameResolutionTest::make_constructor("Constructor1", std::move(constructor_fields)));
  std::vector<ast::Enum::Rule::InputPattern> inputs;
  inputs.emplace_back(ast::Star {});
  std::vector<ast::Enum::Rule> patterns;
  patterns.emplace_back(
      ast::Enum::Rule {.inputs = std::move(inputs), .outputs = std::move(outputs)});

  ast::Enum enum_b               = NameResolutionTest::make_enum("B", {}, std::move(patterns));
  ast.enums[InternedString("B")] = std::move(enum_b);

  std::unordered_set<std::string> expected_errors {
      "Re-declaration of variable: \"field1\"",
      "Re-declaration of variable: \"field1\"",
      "Re-declaration of variable: \"field2\""};

  checker::ErrorList errors = checker::NameResolutionChecker()(ast);

  ASSERT_EQ(errors.size(), 3);

  for (auto &error : errors) {
    EXPECT_TRUE(expected_errors.contains(error.message));
  }
}

TEST(NameResolutionTest, UnknownConstructor) {
  ast::AST ast;

  std::vector<ast::Constructor> enum1_constructors;
  enum1_constructors.emplace_back(NameResolutionTest::make_constructor("Constructor1", {}));

  std::vector<ast::Enum::Rule> enum1_patterns;
  enum1_patterns.emplace_back(NameResolutionTest::make_rule({}, std::move(enum1_constructors)));

  ast::Enum enum1 = NameResolutionTest::make_enum("A", {}, std::move(enum1_patterns));

  ast.enums[InternedString("A")] = std::move(enum1);

  std::vector<ast::TypedVariable> enum2_fields;
  enum2_fields.emplace_back(NameResolutionTest::make_simple_typed_variable("field1", "A"));

  std::vector<ast::Constructor> enum2_outputs;
  enum2_outputs.emplace_back(
      NameResolutionTest::make_constructor("Constructor3", std::move(enum2_fields)));

  std::vector<ast::Enum::Rule::InputPattern> enum2_inputs;
  enum2_inputs.emplace_back(NameResolutionTest::make_constructed_value("Constructor2", {}));

  std::vector<ast::Enum::Rule> enum2_pattern;
  enum2_pattern.emplace_back(
      NameResolutionTest::make_rule(std::move(enum2_inputs), std::move(enum2_outputs)));

  std::vector<ast::TypedVariable> enum2_dependencies;
  enum2_dependencies.emplace_back(NameResolutionTest::make_simple_typed_variable("a", "A"));

  ast::Enum enum2 =
      NameResolutionTest::make_enum("B", std::move(enum2_dependencies), std::move(enum2_pattern));

  ast.enums[InternedString("B")] = std::move(enum2);

  std::unordered_set<std::string> expected_errors {"Undefined constructor: \"Constructor2\""};

  checker::ErrorList errors = checker::NameResolutionChecker()(ast);

  ASSERT_EQ(errors.size(), 1);

  for (auto &error : errors) {
    EXPECT_TRUE(expected_errors.contains(error.message));
  }
}

TEST(NameResolutionTest, UnknownFieldOfConstructor) {
  ast::AST ast;

  std::vector<ast::TypedVariable> enum1_fields;
  enum1_fields.emplace_back(NameResolutionTest::make_simple_typed_variable("field1", "Bool"));

  std::vector<ast::Constructor> enum1_constructors;
  enum1_constructors.emplace_back(
      NameResolutionTest::make_constructor("C1", std::move(enum1_fields)));

  std::vector<ast::Enum::Rule> enum1_patterns;
  enum1_patterns.emplace_back(NameResolutionTest::make_rule({}, std::move(enum1_constructors)));

  ast::Enum enum1 = NameResolutionTest::make_enum("A", {}, std::move(enum1_patterns));

  ast.enums[InternedString("A")] = std::move(enum1);

  std::vector<ast::TypedVariable> enum2_fields;

  std::vector<std::pair<InternedString, std::unique_ptr<ast::Expression>>> enum2_fields_assigment;
  enum2_fields_assigment.emplace_back(NameResolutionTest::make_field_assigment("field1", false));
  enum2_fields_assigment.emplace_back(
      NameResolutionTest::make_field_assigment("field2", std::string("string")));

  std::vector<ast::Enum::Rule::InputPattern> enum2_inputs;
  enum2_inputs.emplace_back(
      NameResolutionTest::make_constructed_value("C1", std::move(enum2_fields_assigment)));

  std::vector<ast::Enum::Rule> enum2_pattern;
  enum2_pattern.emplace_back(NameResolutionTest::make_rule(std::move(enum2_inputs), {}));

  std::vector<ast::TypedVariable> enum2_dependencies;
  enum2_dependencies.emplace_back(NameResolutionTest::make_simple_typed_variable("a", "A"));

  ast::Enum enum2 =
      NameResolutionTest::make_enum("B", std::move(enum2_dependencies), std::move(enum2_pattern));

  ast.enums[InternedString("B")] = std::move(enum2);

  std::unordered_set<std::string> expected_errors {"No field with name field2 in constructor C1"};

  checker::ErrorList errors = checker::NameResolutionChecker()(ast);

  ASSERT_EQ(errors.size(), 1);

  for (auto &error : errors) {
    EXPECT_TRUE(expected_errors.contains(error.message));
  }
}

} // namespace dbuf
