#pragma once

#include "core/ast/ast.h"
#include "core/codegen/kotlin_target/kotlin_printer.h"
#include "core/interning/interned_string.h"

#include <string_view>
#include <unordered_map>
#include <unordered_set>

namespace dbuf::gen::kotlin {

/**
 * @brief scope, like '(...)'
 *
 */
class ParenthesesScope : public Scope {
public:
  explicit ParenthesesScope(Printer &printer);
  void Start() override;
  void End() override;
  ~ParenthesesScope() override;
};

/**
 * @brief scope, like '{...}'
 *
 */
class BracesScope : public Scope {
public:
  explicit BracesScope(Printer &printer);
  void Start() override;
  void End() override;
  ~BracesScope() override;
};

/**
 * @brief Prints typed variable, like `x: Int`
 *
 */
class PrintableVariable : public PrintableObject {
public:
  explicit PrintableVariable(const ast::TypedVariable &typed_variable);
  void Print(Printer &printer) const override;
  ~PrintableVariable() override = default;

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
 * @brief decides wheter property has custom type or not and prints it
 *
 */
class Property : public PrintableObject {
public:
  explicit Property(const ast::TypedVariable &typed_variable);
  void Print(Printer &printer) const override;
  ~Property() override = default;

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
 * @brief Prints secondary constructor for messages and constructors
 *
 */
class SecondaryConstructor : public PrintableObject {
public:
  SecondaryConstructor(const ast::TypeWithFields &fields, const ast::DependentType &dependency);
  void Print(Printer &printer) const override;
  ~SecondaryConstructor() override = default;

private:
  const ast::TypeWithFields &fields_;
  const ast::DependentType &dependency_;
};

/**
 * @brief calls `fun equals()` for default types and `fun sameFields()` for dbuf types
 *
 * @param not_expression defines whether should use == or != (default is ==).
 *
 */
class SmartEqual : public PrintableObject {
public:
  explicit SmartEqual(const ast::Identifier &indentifiable, bool not_expression = false);
  void Print(Printer &printer) const override;
  ~SmartEqual() override = default;

private:
  const ast::Identifier &indentifiable_;
  bool not_expression_;
};

/**
 * @brief Prints all checks for dependency
 *
 * @param dependency_name name of property witch type has dependencies
 *
 * @param dependency_propery field of `dependency_name`
 *
 * @param dependency_type type of `dependency_name.dependency_property`
 *
 * @param expression expected expression for `dependency_name.dependency_property`
 *
 */
class DependencyCheck : public PrintableObject {
public:
  DependencyCheck(
      const dbuf::InternedString &dependency_name,
      const dbuf::InternedString &dependency_property,
      const ast::Identifier &dependency_type,
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
  const ast::Identifier &dependency_type_;
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
 * @brief Prints method `fun equals()` for message and constructor
 *
 */
class ClassEquals : public PrintableObject {
public:
  ClassEquals(const ast::Identifiable &target, const ast::TypeWithFields &fields, const ast::DependentType &dependency);
  void Print(Printer &printer) const override;
  ~ClassEquals() override = default;

private:
  const ast::Identifiable &target_;
  const ast::TypeWithFields &fields_;
  const ast::DependentType &dependency_;
};

/**
 * @brief Prints method `infix internal fun sameFields()` for message and constructor
 *
 */
class ClassSameFields : public PrintableObject {
public:
  ClassSameFields(const ast::Identifiable &target, const ast::TypeWithFields &fields);
  void Print(Printer &printer) const override;
  ~ClassSameFields() override = default;

private:
  const ast::Identifiable &target_;
  const ast::TypeWithFields &fields_;
};

/**
 * @brief Prints method `infix internal fun notSameFields()` for all classes
 *
 */
class ClassNotSameFields : public PrintableObject {
public:
  ClassNotSameFields() = default;
  void Print(Printer &printer) const override;
  ~ClassNotSameFields() override = default;
};

/**
 * @brief Prints method `override fun toString()` for all classes
 *
 */
class ClassToString : public PrintableObject {
public:
  ClassToString() = default;
  void Print(Printer &printer) const override;
  ~ClassToString() override = default;
};

/**
 * @brief Prints method `fun toString(depth: UInt())` for message and constructor
 * (implementation of `override fun toString()`)
 *
 */
class ClassToStringImpl : public PrintableObject {
public:
  ClassToStringImpl(
      const ast::Identifiable &target,
      const ast::TypeWithFields &fields,
      const ast::DependentType &dependency);
  void Print(Printer &printer) const override;
  ~ClassToStringImpl() override = default;

private:
  static void PrintValue(SeparatablePrinter<const char *> &sprinter, const ast::TypedVariable &variable);

  const ast::Identifiable &target_;
  const ast::TypeWithFields &fields_;
  const ast::DependentType &dependency_;
};

/**
 * @brief Prints default value of `variable` type
 *
 */
class DefaultValue : public PrintableObject {
public:
  explicit DefaultValue(const ast::TypedVariable &variable);
  void Print(Printer &printer) const override;
  ~DefaultValue() override = default;

private:
  const ast::TypedVariable &variable_;
};

/**
 * @brief Prints method `fun default()` for companion of message class
 *
 */
class DefaultMessageCompanion : public PrintableObject {
public:
  explicit DefaultMessageCompanion(const ast::Message &message);
  void Print(Printer &printer) const override;
  ~DefaultMessageCompanion() override = default;

private:
  const ast::Message &message_;
};

/**
 * @brief Prints method `fun make(...)` for companion of message class
 *
 */
class MakeMessageCompanion : public PrintableObject {
public:
  explicit MakeMessageCompanion(const ast::Message &message);
  void Print(Printer &printer) const override;
  ~MakeMessageCompanion() override = default;

private:
  const ast::Message &message_;
};

/**
 * @brief Prints companion object for message
 *
 */
class MessageCompanion : public PrintableObject {
public:
  explicit MessageCompanion(const ast::Message &message);
  void Print(Printer &printer) const override;
  ~MessageCompanion() override = default;

private:
  const ast::Message &message_;
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
 * @brief Prints method `fun default()` for companion of constructor class
 *
 */
class DefaultConstructorCompanion : public PrintableObject {
public:
  DefaultConstructorCompanion(const ast::Constructor &constructor, const ast::DependentType &dependencies);
  void Print(Printer &printer) const override;
  ~DefaultConstructorCompanion() override = default;

private:
  const ast::Constructor &constructor_;
  const ast::DependentType &dependencies_;
};

/**
 * @brief Prints method `fun make()` for companion of constructor class
 *
 */
class MakeConstructorCompanion : public PrintableObject {
public:
  MakeConstructorCompanion(const ast::Constructor &constructor, const dbuf::InternedString &enum_name);
  void Print(Printer &printer) const override;
  ~MakeConstructorCompanion() override = default;

private:
  const ast::Constructor &constructor_;
  const dbuf::InternedString &enum_name_;
};

/**
 * @brief Prints companion object for constructor class
 *
 */
class ConstructorCompanion : public PrintableObject {
public:
  ConstructorCompanion(
      const ast::Constructor &constructor,
      const ast::DependentType &dependencies,
      const dbuf::InternedString &enum_name);
  void Print(Printer &printer) const override;
  ~ConstructorCompanion() override = default;

private:
  const ast::Constructor &constructor_;
  const ast::DependentType &dependencies_;
  const dbuf::InternedString &enum_name_;
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

  /**
   * @brief Error message pattern
   *
   */
  static const std::string_view kErrorMessage;

private:
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
 * @brief Prints secondary constructor for enum
 *
 */
class EnumSecondaryConstructor : public PrintableObject {
public:
  explicit EnumSecondaryConstructor(const ast::DependentType &dependency);
  void Print(Printer &printer) const override;
  ~EnumSecondaryConstructor() override = default;

private:
  const ast::DependentType &dependency_;
};

/**
 * @brief Prints `fun equals()` for enum
 *
 */
class EnumEquals : public PrintableObject {
public:
  explicit EnumEquals(const ast::Enum &ast_enum);
  void Print(Printer &printer) const override;
  ~EnumEquals() override = default;

private:
  const ast::Enum &ast_enum_;
};

/**
 * @brief Prints method `fun toString(depth: UInt())` for enum
 * (implementation of `override fun toString()`)
 *
 */
class EnumToStringImpl : public PrintableObject {
public:
  explicit EnumToStringImpl(const ast::Enum &ast_enum);
  void Print(Printer &printer) const override;
  ~EnumToStringImpl() override = default;

private:
  const ast::Enum &ast_enum_;
};

/**
 * @brief Prints method `infix internal fun sameFields()` for enum
 *
 */
class EnumSameFields : public PrintableObject {
public:
  explicit EnumSameFields(const ast::Enum &ast_enum);
  void Print(Printer &printer) const override;
  ~EnumSameFields() override = default;

private:
  const ast::Enum &ast_enum_;
};

/**
 * @brief Prints method `fun default()` for companion of enum class
 *
 */
class DefaultEnumCompanion : public PrintableObject {
public:
  explicit DefaultEnumCompanion(const ast::Enum &ast_enum);
  void Print(Printer &printer) const override;
  ~DefaultEnumCompanion() override = default;

private:
  const ast::Enum &ast_enum_;
};

/**
 * @brief Prints companion object for enum class
 *
 */
class EnumCompanion : public PrintableObject {
public:
  explicit EnumCompanion(const ast::Enum &ast_enum);
  void Print(Printer &printer) const override;
  ~EnumCompanion() override = default;

private:
  const ast::Enum &ast_enum_;
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

  /**
   * @brief name of property inside enum class. By default it is `inside`
   *
   */
  static const std::string_view kPropertyName;

private:
  const ast::Enum &ast_enum_;
  const ast::AST *tree_;
};

} // namespace dbuf::gen::kotlin
