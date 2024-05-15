#pragma once

#include "core/ast/ast.h"
#include "core/ast/expression.h"
#include "core/codegen/sharp_target/sharp_constants.h"
#include "core/interning/interned_string.h"

#include <array>
#include <fstream>
#include <memory>
#include <unordered_set>
#include <vector>

namespace dbuf::gen {

class SharpTypes {
public:
  static constexpr std::string ConstructSharpType(const std::string &dbuf_type);

private:
  static constexpr unsigned int HashString(const std::string &str);

  static constexpr const char *kTypes[] = {"bool", "float", "int", "string", "unsigned"};
};

class SharpPrinter {
public:
  SharpPrinter() = default;

  explicit SharpPrinter(std::shared_ptr<std::ofstream> output);

  void InitFile();

  void CompleteFile();

  void PrintClassBegin(const std::string &name);

  void PrintClassEnd();

  void PrintConstructorBegin(
      const std::string &name,
      const std::vector<std::pair<InternedString, InternedString>> &dependent_variables,
      bool print_vars = true);

  void PrintConstructorEnd();

  void PrintBaseEnumFields();

  void PrintBaseEnumConstructor();

  void PrintBaseEnumCheck();

  void PrintTypeExpression(
      const ast::TypeExpression &expression,
      bool is_public,
      bool as_dependency = false,
      bool need_access   = true);

  void PrintBinaryExpressionBegin();

  void PrintBinaryExpressionType(const ast::BinaryExpression &binary_expr);

  void PrintBinaryExpressionEnd();

  void PrintUnaryExpressionType(const ast::UnaryExpression &unary_expr);

  void PrintConstructedValueBegin(const ast::ConstructedValue &constr_value);

  void PrintConstructedValueEnd();

  void PrintValue(const ast::Value &value);

  void PrintVarAccess(const ast::VarAccess &var_access);

private:
  static std::string GetVariableName(const ast::TypedVariable &var);

  static std::string GetVariableType(const ast::TypedVariable &var);

  static std::string GetExpressionType(const ast::TypeExpression &expr);

  std::shared_ptr<std::ofstream> out_ = nullptr;

  SharpTypes type_constructor_;
};

} // namespace dbuf::gen
