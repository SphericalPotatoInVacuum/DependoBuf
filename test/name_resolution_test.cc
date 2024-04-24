/*
This file is part of DependoBuf project.

Copyright (C) 2023 Alexander Bogdanov, Alice Vernigor

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/
#include "core/ast/ast.h"
#include "core/ast/expression.h"
#include "core/checker/common.h"
#include "core/checker/name_resolution_checker.h"
#include "core/interning/interned_string.h"
#include "location.hh"

#include <gtest/gtest.h>
#include <memory>
#include <unordered_set>
#include <utility>
#include <vector>

namespace dbuf {

class NameResolutionTest : public ::testing::TestSuite {
public:
  static ast::TypedVariable make_typed_variable(std::string &&name, ast::TypeExpression &&type_expression) {
    return ast::TypedVariable {
        {InternedString(std::move(name))},
        std::move(type_expression),
    };
  }

  template <typename T>
  static std::pair<ast::Identifier, std::shared_ptr<const ast::Expression>>
  make_field_assigment(std::string &&field_name, T value) {
    std::shared_ptr<const ast::Expression> ptr =
        std::make_unique<ast::Expression>(ast::Value(ast::ScalarValue<T> {{parser::location()}, value}));
    return std::make_pair(
        ast::Identifier {parser::location(), InternedString(field_name)},
        std::make_unique<ast::Expression>(ast::ScalarValue<T> {{parser::location()}, value}));
  }

  static ast::VarAccess make_var_access(std::string &&var_identifier, std::vector<std::string> &&field_identifiers) {
    ast::VarAccess result {{{parser::location()}, InternedString(std::move(var_identifier))}};

    std::vector<ast::Identifier> fields(field_identifiers.size());
    for (auto &&field_identifier : field_identifiers) {
      fields.emplace_back(ast::Identifier {parser::location(), InternedString(field_identifier)});
    }
    result.field_identifiers = std::move(fields);

    return result;
  }

  static ast::ConstructedValue make_constructed_value(
      std::string &&constructor_identifier,
      std::vector<std::pair<ast::Identifier, std::shared_ptr<const ast::Expression>>> &&fields) {
    return ast::ConstructedValue {
        {parser::location()},
        ast::Identifier {{parser::location()}, InternedString(std::move(constructor_identifier))},
        std::move(fields)};
  }

  static ast::TypedVariable make_simple_typed_variable(std::string &&name, std::string &&type) {
    return ast::TypedVariable {
        {InternedString(std::move(name))},
        ast::TypeExpression {
            {parser::location()},
            {parser::location(), InternedString(std::move(type))},
        }};
  }

  static ast::TypeExpression
  make_type_expression(std::string &&name, std::vector<std::shared_ptr<const ast::Expression>> &&parameters) {
    return ast::TypeExpression {
        {parser::location()},
        {parser::location(), InternedString(std::move(name))},
        std::move(parameters)};
  }

  static ast::Message make_message(
      std::string &&name,
      std::vector<ast::TypedVariable> &&type_dependencies,
      std::vector<ast::TypedVariable> &&fields) {
    return ast::Message {
        {{parser::location(), InternedString(std::move(name))}},
        {std::move(type_dependencies)},
        {std::move(fields)}};
  }

  static ast::Constructor make_constructor(std::string &&name, std::vector<ast::TypedVariable> &&fields) {
    return ast::Constructor {{parser::location(), InternedString(std::move(name))}, {std::move(fields)}};
  }

  static ast::Enum make_enum(
      std::string &&name,
      std::vector<ast::TypedVariable> &&type_dependencies,
      std::vector<ast::Enum::Rule> &&pattern_mapping) {
    return ast::Enum {
        {{parser::location(), InternedString(std::move(name))}},
        {std::move(type_dependencies)},
        {std::move(pattern_mapping)}};
  }

  static ast::Enum::Rule
  make_rule(std::vector<ast::Enum::Rule::InputPattern> &&inputs, std::vector<ast::Constructor> &&outputs) {
    return ast::Enum::Rule {.inputs = std::move(inputs), .outputs = std::move(outputs)};
  }
};

TEST(NameResolutionTest, GlobalNamesRepetition) {
  ast::AST ast;

  ast::Message message_a         = NameResolutionTest::make_message("A", {}, {});
  ast.types[InternedString("A")] = std::move(message_a);

  ast::Constructor constructor_a = NameResolutionTest::make_constructor("A", {});
  std::vector<ast::Constructor> outputs;
  outputs.emplace_back(std::move(constructor_a));

  std::vector<ast::Enum::Rule> patterns;
  patterns.emplace_back(NameResolutionTest::make_rule({}, std::move(outputs)));

  ast::Enum enum_a               = NameResolutionTest::make_enum("A", {}, std::move(patterns));
  ast.types[InternedString("A")] = std::move(enum_a);

  std::string expected_error = "Re-declaration of constructor: \"A\"";

  std::optional<checker::Error> error = checker::NameResolutionChecker()(ast);

  ASSERT_TRUE(error.has_value());

  ASSERT_TRUE(error->message == expected_error);
}

TEST(NameResolutionTest, ConstructorRedeclaration) {
  ast::AST ast;

  ast::Constructor constructor_a = NameResolutionTest::make_constructor("Constructor1", {});
  std::vector<ast::Constructor> outputs_a;
  outputs_a.emplace_back(std::move(constructor_a));

  std::vector<ast::Enum::Rule> patterns_a;
  patterns_a.emplace_back(NameResolutionTest::make_rule({}, std::move(outputs_a)));

  ast::Enum enum_a               = NameResolutionTest::make_enum("A", {}, std::move(patterns_a));
  ast.types[InternedString("A")] = std::move(enum_a);

  ast::Constructor constructor_b = NameResolutionTest::make_constructor("Constructor1", {});
  std::vector<ast::Constructor> outputs_b;
  outputs_b.emplace_back(std::move(constructor_b));

  std::vector<ast::Enum::Rule> patterns_b;
  patterns_b.emplace_back(NameResolutionTest::make_rule({}, std::move(outputs_b)));

  ast::Enum enum_b               = NameResolutionTest::make_enum("B", {}, std::move(patterns_b));
  ast.types[InternedString("B")] = std::move(enum_b);

  std::string expected_error = "Re-declaration of constructor: \"Constructor1\"";

  std::optional<checker::Error> error = checker::NameResolutionChecker()(ast);

  ASSERT_TRUE(error.has_value());

  ASSERT_TRUE(error->message == expected_error);
}

TEST(NameResolutionTest, UnknownTypename) {
  ast::AST ast;

  std::vector<ast::TypedVariable> dependencies;
  dependencies.emplace_back(NameResolutionTest::make_simple_typed_variable("a", "B"));

  std::vector<ast::TypedVariable> fields;
  fields.emplace_back(NameResolutionTest::make_simple_typed_variable("field1", "C"));

  ast::Message message_a         = NameResolutionTest::make_message("A", std::move(dependencies), std::move(fields));
  ast.types[InternedString("A")] = std::move(message_a);

  std::string expected_error = "Undefined type name: \"B\" at 1.1";

  std::optional<checker::Error> error = checker::NameResolutionChecker()(ast);

  ASSERT_TRUE(error.has_value());

  ASSERT_TRUE(error->message == expected_error);
}

TEST(NameResolutionTest, FieldRedeclaration) {
  ast::AST ast;

  std::vector<ast::TypedVariable> message_fields;
  message_fields.emplace_back(NameResolutionTest::make_simple_typed_variable("field1", "Int"));
  message_fields.emplace_back(NameResolutionTest::make_simple_typed_variable("field1", "Bool"));
  message_fields.emplace_back(NameResolutionTest::make_simple_typed_variable("field1", "Int"));
  ast::Message message_a         = NameResolutionTest::make_message("A", {}, std::move(message_fields));
  ast.types[InternedString("A")] = std::move(message_a);

  std::vector<ast::TypedVariable> constructor_fields;
  constructor_fields.emplace_back(NameResolutionTest::make_simple_typed_variable("field2", "Int"));
  constructor_fields.emplace_back(NameResolutionTest::make_simple_typed_variable("field2", "Bool"));

  std::vector<ast::Constructor> outputs;
  outputs.emplace_back(NameResolutionTest::make_constructor("Constructor1", std::move(constructor_fields)));
  std::vector<ast::Enum::Rule::InputPattern> inputs;
  inputs.emplace_back(ast::Star {parser::location()});
  std::vector<ast::Enum::Rule> patterns;
  patterns.emplace_back(ast::Enum::Rule {.inputs = std::move(inputs), .outputs = std::move(outputs)});

  ast::Enum enum_b               = NameResolutionTest::make_enum("B", {}, std::move(patterns));
  ast.types[InternedString("B")] = std::move(enum_b);

  std::string expected_error = "Re-declaration of variable: \"field2\"";

  std::optional<checker::Error> error = checker::NameResolutionChecker()(ast);

  ASSERT_TRUE(error.has_value());

  ASSERT_TRUE(error->message == expected_error);
}

TEST(NameResolutionTest, UnknownConstructor) {
  ast::AST ast;

  std::vector<ast::Constructor> enum1_constructors;
  enum1_constructors.emplace_back(NameResolutionTest::make_constructor("Constructor1", {}));

  std::vector<ast::Enum::Rule> enum1_patterns;
  enum1_patterns.emplace_back(NameResolutionTest::make_rule({}, std::move(enum1_constructors)));

  ast::Enum enum1 = NameResolutionTest::make_enum("A", {}, std::move(enum1_patterns));

  ast.types[InternedString("A")] = std::move(enum1);

  std::vector<ast::TypedVariable> enum2_fields;
  enum2_fields.emplace_back(NameResolutionTest::make_simple_typed_variable("field1", "A"));

  std::vector<ast::Constructor> enum2_outputs;
  enum2_outputs.emplace_back(NameResolutionTest::make_constructor("Constructor3", std::move(enum2_fields)));

  std::vector<ast::Enum::Rule::InputPattern> enum2_inputs;
  enum2_inputs.emplace_back(NameResolutionTest::make_constructed_value("Constructor2", {}));

  std::vector<ast::Enum::Rule> enum2_pattern;
  enum2_pattern.emplace_back(NameResolutionTest::make_rule(std::move(enum2_inputs), std::move(enum2_outputs)));

  std::vector<ast::TypedVariable> enum2_dependencies;
  enum2_dependencies.emplace_back(NameResolutionTest::make_simple_typed_variable("a", "A"));

  ast::Enum enum2 = NameResolutionTest::make_enum("B", std::move(enum2_dependencies), std::move(enum2_pattern));

  ast.types[InternedString("B")] = std::move(enum2);

  std::string expected_error = "Undefined constructor: \"Constructor2\"";

  std::optional<checker::Error> error = checker::NameResolutionChecker()(ast);

  ASSERT_TRUE(error.has_value());

  ASSERT_TRUE(error->message == expected_error);
}

TEST(NameResolutionTest, UnknownFieldOfConstructor) {
  ast::AST ast;

  std::vector<ast::TypedVariable> enum1_fields;
  enum1_fields.emplace_back(NameResolutionTest::make_simple_typed_variable("field1", "Bool"));

  std::vector<ast::Constructor> enum1_constructors;
  enum1_constructors.emplace_back(NameResolutionTest::make_constructor("C1", std::move(enum1_fields)));

  std::vector<ast::Enum::Rule> enum1_patterns;
  enum1_patterns.emplace_back(NameResolutionTest::make_rule({}, std::move(enum1_constructors)));

  ast::Enum enum1 = NameResolutionTest::make_enum("A", {}, std::move(enum1_patterns));

  ast.types[InternedString("A")] = std::move(enum1);

  std::vector<ast::TypedVariable> enum2_fields;

  std::vector<std::pair<ast::Identifier, std::shared_ptr<const ast::Expression>>> enum2_fields_assigment;
  enum2_fields_assigment.emplace_back(NameResolutionTest::make_field_assigment("field1", false));
  enum2_fields_assigment.emplace_back(NameResolutionTest::make_field_assigment("field2", std::string("string")));

  std::vector<ast::Enum::Rule::InputPattern> enum2_inputs;
  enum2_inputs.emplace_back(NameResolutionTest::make_constructed_value("C1", std::move(enum2_fields_assigment)));

  std::vector<ast::Enum::Rule> enum2_pattern;
  enum2_pattern.emplace_back(NameResolutionTest::make_rule(std::move(enum2_inputs), {}));

  std::vector<ast::TypedVariable> enum2_dependencies;
  enum2_dependencies.emplace_back(NameResolutionTest::make_simple_typed_variable("a", "A"));

  ast::Enum enum2 = NameResolutionTest::make_enum("B", std::move(enum2_dependencies), std::move(enum2_pattern));

  ast.types[InternedString("B")] = std::move(enum2);

  std::string expected_error = "No field with name field2 in constructor C1";

  std::optional<checker::Error> error = checker::NameResolutionChecker()(ast);

  ASSERT_TRUE(error.has_value());

  ASSERT_TRUE(error->message == expected_error);
}

TEST(NameResolutionTest, UnknownVariable) {
  ast::AST ast;

  std::vector<ast::TypedVariable> message_vec_dependencies;
  message_vec_dependencies.emplace_back(NameResolutionTest::make_simple_typed_variable("i", "Int"));
  message_vec_dependencies.emplace_back(NameResolutionTest::make_simple_typed_variable("n", "Int"));
  ast::Message message_vec         = NameResolutionTest::make_message("Vec", std::move(message_vec_dependencies), {});
  ast.types[InternedString("Vec")] = std::move(message_vec);

  std::vector<std::shared_ptr<const ast::Expression>> parameters;
  parameters.emplace_back(std::make_unique<ast::Expression>(NameResolutionTest::make_var_access("a", {})));
  parameters.emplace_back(std::make_unique<ast::Expression>(NameResolutionTest::make_var_access("b", {})));
  ast::TypeExpression type_expression = NameResolutionTest::make_type_expression("Vec", std::move(parameters));

  std::vector<ast::TypedVariable> message_a_fields;
  message_a_fields.emplace_back(NameResolutionTest::make_typed_variable("field1", std::move(type_expression)));

  ast::Message message_a         = NameResolutionTest::make_message("A", {}, std::move(message_a_fields));
  ast.types[InternedString("A")] = std::move(message_a);

  std::string expected_error = "Undefined variable: \"a\" at 1.1";

  std::optional<checker::Error> error = checker::NameResolutionChecker()(ast);

  ASSERT_TRUE(error.has_value());

  ASSERT_TRUE(error->message == expected_error);
}

} // namespace dbuf
