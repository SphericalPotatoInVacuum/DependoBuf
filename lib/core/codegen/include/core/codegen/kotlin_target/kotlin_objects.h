#pragma once

#include "core/ast/ast.h"
#include "core/codegen/kotlin_target/kotlin_printer.h"
#include "core/interning/interned_string.h"

#include <string_view>
#include <unordered_map>
#include <unordered_set>

namespace dbuf::gen::kotlin {

/**
 * @brief Prints one property of primary constructor, like `val x: Int`
 *
 */
class ConstructorParameter : public PrintableObject {
public:
  explicit ConstructorParameter(const ast::TypedVariable &typed_variable);
  void Print(Printer &printer) const override;
  ~ConstructorParameter() override = default;

private:
  const ast::TypedVariable &typed_variable_;
};

/**
 * @brief Prints primary constructor for class, like `(val x: Int, val y: String)`
 *
 */
class Constructor : public PrintableObject {
public:
  explicit Constructor(const ast::DependentType &dependent_type);
  void Print(Printer &printer) const override;
  ~Constructor() override = default;

private:
  const ast::DependentType &dependent_type_;
};

/**
 * @brief Prints property with default types, like `var x: Int = 0`
 *
 */
class DefaultTypeProperty : public PrintableObject {
public:
  explicit DefaultTypeProperty(const ast::TypedVariable &typed_variable);
  void Print(Printer &printer) const override;
  ~DefaultTypeProperty() override = default;

private:
  const ast::TypedVariable &typed_variable_;
};

/**
 * @brief Prints property with custom type, like `lateinit var x: Message`
 *
 */
class CustomTypeProperty : public PrintableObject {
public:
  explicit CustomTypeProperty(const ast::TypedVariable &typed_variable);
  void Print(Printer &printer) const override;
  ~CustomTypeProperty() override = default;

private:
  const ast::TypedVariable &typed_variable_;
};

/**
 * @brief Prints all class properties
 *
 */
class Properties : public PrintableObject {
public:
  explicit Properties(const ast::TypeWithFields &type_with_fields);
  void Print(Printer &printer) const override;
  ~Properties() override = default;

private:
  const ast::TypeWithFields &type_with_fields_;
};

/**
 * @brief Prints `ast::Expression` object
 *
 * @warning do not support `ast::ConstructedValue`
 *
 * @todo support `ast::ConstructedValue`
 *
 */
class PrintableExpression : public PrintableObject {
public:
  explicit PrintableExpression(const ast::Expression &expression);
  void Print(Printer &printer) const override;
  ~PrintableExpression() override = default;

private:
  const ast::Expression &expression_;
};

/**
 * @brief Prints all checks for dependency
 *
 * @param dependency_name name of property witch type has dependencies
 *
 * @param dependency_propery field of `dependency_name`
 *
 * @param expression expected expression for `dependency_name.dependency_property`
 *
 */
class DependencyCheck : public PrintableObject {
public:
  DependencyCheck(
      const dbuf::InternedString &dependency_name,
      const dbuf::InternedString &dependency_property,
      const ast::Expression &expression);
  void Print(Printer &printer) const override;
  ~DependencyCheck() override = default;

private:
  /**
   * @brief Error message pattern, where `A`, `B`, `C` replaces to other strings
   *
   * @param A replaces to `dependency_name`
   *
   * @param B replaces to `dependency_property`
   *
   * @param C replaces to `expression`
   *
   */
  static const std::string_view kErrorMessage;

  const dbuf::InternedString &dependency_name_;
  const dbuf::InternedString &dependency_property_;
  const ast::Expression &expression_;
};

/**
 * @brief Prints init check for property
 *
 */
class InitCheck : public PrintableObject {
public:
  explicit InitCheck(const std::string_view &property);
  void Print(Printer &printer) const override;
  ~InitCheck() override = default;

private:
  /**
   * @brief Error message pattern, where `A` replaces to `property`
   *
   * @param A replaces to `property`
   *
   */
  static const std::string_view kErrorMessage;

  const std::string_view &property_;
};

/**
 * @brief Prints method `fun check()` for message class
 *
 */
class MessageCheck : public PrintableObject {
public:
  MessageCheck(const ast::Message &message, const ast::AST *tree);
  void Print(Printer &printer) const override;
  ~MessageCheck() override = default;

private:
  const ast::Message &message_;
  const ast::AST *tree_;
};

/**
 * @brief Prints class, that represent message
 *
 */
class PrintableMessage : public PrintableObject {
public:
  PrintableMessage(const ast::Message &message, const ast::AST *tree);
  void Print(Printer &printer) const override;
  ~PrintableMessage() override = default;

private:
  const ast::Message &message_;
  const ast::AST *tree_;
};

/**
 * @brief Prints method `fun check()` for constructor class
 *
 */
class ConstructorCheck : public PrintableObject {
public:
  ConstructorCheck(const ast::Constructor &constructor_, const ast::AST *tree);
  void Print(Printer &printer) const override;
  ~ConstructorCheck() override = default;

private:
  const ast::Constructor &constructor_;
  const ast::AST *tree_;
};

/**
 * @brief Prints class, that represent constructor
 *
 */
class PrintableConstructor : public PrintableObject {
public:
  PrintableConstructor(
      const ast::DependentType &dependent_type,
      const ast::Constructor &constructor,
      const ast::AST *tree);
  void Print(Printer &printer) const override;
  ~PrintableConstructor() override = default;

private:
  const ast::DependentType &dependent_type_;
  const ast::Constructor &constructor_;
  const ast::AST *tree_;
};

/**
 * @brief Prints one check for one enum pattern, like `x == 2`
 *
 */
class EnumStatement : public PrintableObject {
public:
  EnumStatement(const ast::TypedVariable &target, const ast::Value &expect);
  void Print(Printer &printer) const override;
  ~EnumStatement() override = default;

private:
  const ast::TypedVariable &target_;
  const ast::Value &expect_;
};

/**
 * @brief Prints whole check for one enum pattern
 *
 */
class EnumRuleCheck : public PrintableObject {
public:
  EnumRuleCheck(const ast::Enum::Rule &rule, const ast::DependentType &dependent_type);
  void Print(Printer &printer) const override;
  ~EnumRuleCheck() override = default;

private:
  /**
   * @brief Error message pattern
   *
   */
  static const std::string_view kErrorMessage;

  const ast::Enum::Rule &rule_;
  const ast::DependentType &dependent_type_;
};

/**
 * @brief Prints method `fun check()` for enum class
 *
 */
class EnumCheck : public PrintableObject {
public:
  EnumCheck(const ast::Enum &ast_enum, const ast::AST *tree);
  void Print(Printer &printer) const override;
  ~EnumCheck() override = default;

private:
  const ast::Enum &ast_enum_;
  const ast::AST *tree_;
};

/**
 * @brief Prints class, that represent enum
 *
 */
class PrintableEnum : public PrintableObject {
public:
  PrintableEnum(const ast::Enum &ast_enum, const ast::AST *tree);
  void Print(Printer &printer) const override;
  ~PrintableEnum() override = default;

public:
  /**
   * @brief name of property inside enum class. By default it is `lateinit var inside: Any`
   *
   */
  static const std::string_view kPropertyName;

private:
  const ast::Enum &ast_enum_;
  const ast::AST *tree_;
};

} // namespace dbuf::gen::kotlin
