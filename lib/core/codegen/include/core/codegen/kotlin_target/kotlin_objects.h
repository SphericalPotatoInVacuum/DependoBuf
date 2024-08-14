#pragma once

#include "core/ast/ast.h"
#include "core/codegen/kotlin_target/kotlin_printer.h"
#include "core/interning/interned_string.h"

#include <unordered_map>
#include <unordered_set>

namespace dbuf::gen::kotlin {

class ConstructorParameter : public PrintableObject {
public:
  ConstructorParameter(const ast::TypedVariable &typed_variable);
  void Print(Printer &printer) const override;
  ~ConstructorParameter() override = default;

private:
  const ast::TypedVariable &typed_variable_;
};

class Constructor : public PrintableObject {
public:
  Constructor(const ast::DependentType &dependent_type);
  void Print(Printer &printer) const override;
  ~Constructor() override = default;

private:
  const ast::DependentType &dependent_type_;
};

class DefaultTypeProperty : public PrintableObject {
public:
  DefaultTypeProperty(const ast::TypedVariable &typed_variable);
  void Print(Printer &printer) const override;
  ~DefaultTypeProperty() override = default;

private:
  static const std::unordered_map<std::string_view, std::string_view> kTypeMap;
  static const std::unordered_map<std::string_view, std::string_view> kDefaultValues;

  const ast::TypedVariable &typed_variable_;
};

class CustomTypeProperty : public PrintableObject {
public:
  CustomTypeProperty(const ast::TypedVariable &typed_variable);
  void Print(Printer &printer) const override;
  ~CustomTypeProperty() override = default;

private:
  const ast::TypedVariable &typed_variable_;
};

class Properties : public PrintableObject {
public:
  Properties(const ast::TypeWithFields &type_with_fields);
  void Print(Printer &printer) const override;
  ~Properties() override = default;

public:
  static const std::unordered_set<std::string_view> kDefaultTypes;

private:
  const ast::TypeWithFields &type_with_fields_;
};

class PrintableExpression : public PrintableObject {
public:
  PrintableExpression(const ast::Expression &expression);
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
  InitCheck(const dbuf::InternedString &property);
  void Print(Printer &printer) const override;
  ~InitCheck() override = default;

private:
  static const std::string_view kErrorMessage;

  const dbuf::InternedString &property_;
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

} // namespace dbuf::gen::kotlin