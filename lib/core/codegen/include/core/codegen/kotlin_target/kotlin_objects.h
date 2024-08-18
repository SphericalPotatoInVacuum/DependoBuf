#pragma once

#include "core/ast/ast.h"
#include "core/codegen/kotlin_target/kotlin_printer.h"
#include "core/interning/interned_string.h"

#include <string_view>
#include <unordered_map>
#include <unordered_set>

namespace dbuf::gen::kotlin {

class ConstructorParameter : public PrintableObject {
public:
  explicit ConstructorParameter(const ast::TypedVariable &typed_variable);
  void Print(Printer &printer) const override;
  ~ConstructorParameter() override = default;

private:
  const ast::TypedVariable &typed_variable_;
};

class Constructor : public PrintableObject {
public:
  explicit Constructor(const ast::DependentType &dependent_type);
  void Print(Printer &printer) const override;
  ~Constructor() override = default;

private:
  const ast::DependentType &dependent_type_;
};

class DefaultTypeProperty : public PrintableObject {
public:
  explicit DefaultTypeProperty(const ast::TypedVariable &typed_variable);
  void Print(Printer &printer) const override;
  ~DefaultTypeProperty() override = default;

private:
  const ast::TypedVariable &typed_variable_;
};

class CustomTypeProperty : public PrintableObject {
public:
  explicit CustomTypeProperty(const ast::TypedVariable &typed_variable);
  void Print(Printer &printer) const override;
  ~CustomTypeProperty() override = default;

private:
  const ast::TypedVariable &typed_variable_;
};

class Properties : public PrintableObject {
public:
  explicit Properties(const ast::TypeWithFields &type_with_fields);
  void Print(Printer &printer) const override;
  ~Properties() override = default;

private:
  const ast::TypeWithFields &type_with_fields_;
};

class PrintableExpression : public PrintableObject {
public:
  explicit PrintableExpression(const ast::Expression &expression);
  void Print(Printer &printer) const override;
  ~PrintableExpression() override = default;

private:
  const ast::Expression &expression_;
};

class DependencyCheck : public PrintableObject {
public:
  DependencyCheck(
      const dbuf::InternedString &dependency_name,
      const dbuf::InternedString &dependency_property,
      const ast::Expression &expression);
  void Print(Printer &printer) const override;
  ~DependencyCheck() override = default;

private:
  static const std::string_view kErrorMessage;

  const dbuf::InternedString &dependency_name_;
  const dbuf::InternedString &dependency_property_;
  const ast::Expression &expression_;
};

class InitCheck : public PrintableObject {
public:
  explicit InitCheck(const std::string_view &property);
  void Print(Printer &printer) const override;
  ~InitCheck() override = default;

private:
  static const std::string_view kErrorMessage;

  const std::string_view &property_;
};

class MessageCheck : public PrintableObject {
public:
  MessageCheck(const ast::Message &message, const ast::AST *tree);
  void Print(Printer &printer) const override;
  ~MessageCheck() override = default;

private:
  const ast::Message &message_;
  const ast::AST *tree_;
};

class PrintableMessage : public PrintableObject {
public:
  PrintableMessage(const ast::Message &message, const ast::AST *tree);
  void Print(Printer &printer) const override;
  ~PrintableMessage() override = default;

private:
  const ast::Message &message_;
  const ast::AST *tree_;
};

class ConstructorCheck : public PrintableObject {
public:
  ConstructorCheck(const ast::Constructor &constructor_, const ast::AST *tree);
  void Print(Printer &printer) const override;
  ~ConstructorCheck() override = default;

private:
  const ast::Constructor &constructor_;
  const ast::AST *tree_;
};

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

class EnumStatement : public PrintableObject {
public:
  EnumStatement(const ast::TypedVariable &target, const ast::Value &expect);
  void Print(Printer &printer) const override;
  ~EnumStatement() override = default;

private:
  const ast::TypedVariable &target_;
  const ast::Value &expect_;
};

class EnumRuleCheck : public PrintableObject {
public:
  EnumRuleCheck(const ast::Enum::Rule &rule, const ast::DependentType &dependent_type);
  void Print(Printer &printer) const override;
  ~EnumRuleCheck() override = default;

private:
  static const std::string_view kErrorMessage;

  const ast::Enum::Rule &rule_;
  const ast::DependentType &dependent_type_;
};

class EnumCheck : public PrintableObject {
public:
  EnumCheck(const ast::Enum &ast_enum, const ast::AST *tree);
  void Print(Printer &printer) const override;
  ~EnumCheck() override = default;

private:
  const ast::Enum &ast_enum_;
  const ast::AST *tree_;
};

class PrintableEnum : public PrintableObject {
public:
  PrintableEnum(const ast::Enum &ast_enum, const ast::AST *tree);
  void Print(Printer &printer) const override;
  ~PrintableEnum() override = default;

public:
  static const std::string_view kPropertyName;

private:
  const ast::Enum &ast_enum_;
  const ast::AST *tree_;
};

} // namespace dbuf::gen::kotlin
