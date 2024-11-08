#include "core/codegen/TypeScript_target/TypeScript_print.h"

#include "core/ast/expression.h"

namespace dbuf::gen {

constexpr std::string TypeScriptTypes::ConstructTypeScriptType(const std::string &dbuf_type) {
  if (dbuf_type == "Bool") {
    return kTypes[0];
  }
  if (dbuf_type == "Float") {
    return kTypes[1];
  }
  if (dbuf_type == "Int") {
    return kTypes[2];
  }
  if (dbuf_type == "String") {
    return kTypes[3];
  }
  if (dbuf_type == "Unsigned") {
    return kTypes[4];
  }
  return dbuf_type;
}

TypeScriptPrinter::TypeScriptPrinter(std::shared_ptr<std::ofstream> output) {
  out_ = std::move(output);
}

void TypeScriptPrinter::InitFile() {
  *out_ << kReadme << std::endl;
  *out_ << kIncludedLibraries << std::endl;
  *out_ << kOpenNamespace << std::endl;
}

void TypeScriptPrinter::CompleteFile() {
  *out_ << kCloseNamespace << std::endl;
}

// Class
void TypeScriptPrinter::PrintClassBegin(const std::string &name) {
  *out_ << "public class " << name << " {\n";
}

void TypeScriptPrinter::PrintClassEnd() {
  *out_ << "}\n\n";
}

void TypeScriptPrinter::PrintConstructorBegin(
    const std::string &name,
    const std::vector<std::pair<InternedString, InternedString>> &dependent_variables,
    bool print_vars) {
  *out_ << "\n\tpublic " << name << "(";
  bool first = true;
  for (const auto &dependent_var : dependent_variables) {
    if (!first) {
      *out_ << ", ";
    }
    *out_ << type_constructor_.ConstructTypeScriptType(dependent_var.second.GetString()) << " " << dependent_var.first
          << "_";
    first = false;
  }
  *out_ << ") {\n";

  if (print_vars) {
    for (const auto &dependent_var : dependent_variables) {
      *out_ << "\t\t" << dependent_var.first << " = " << dependent_var.first << "_;\n";
    }
  }
}

void TypeScriptPrinter::PrintConstructorEnd() {
  *out_ << "\t}\n";
}

void TypeScriptPrinter::PrintBaseEnumFields() {
  *out_ << kBaseEnumFields << "\n";
}

// TypeExpression
void TypeScriptPrinter::PrintTypeExpression(
    const ast::TypeExpression &expression,
    bool is_public,
    bool as_dependency,
    bool need_access) {
  std::string access;
  if (need_access) {
    access = is_public ? "\tpublic " : "\tprivate ";
  }
  const std::string &readonly = as_dependency ? "readonly " : "";
  const std::string &type     = GetExpressionType(expression);
  *out_ << access << readonly << type_constructor_.ConstructTypeScriptType(type);
}

// BinaryExpression
void TypeScriptPrinter::PrintBinaryExpressionBegin() {
  *out_ << "(";
}

void TypeScriptPrinter::PrintBinaryExpressionType(const ast::BinaryExpression &binary_expr) {
  *out_ << " " << static_cast<char>(binary_expr.type) << " ";
}

void TypeScriptPrinter::PrintBinaryExpressionEnd() {
  *out_ << ")";
}

// UnaryExpression
void TypeScriptPrinter::PrintUnaryExpressionType(const ast::UnaryExpression &unary_expr) {
  *out_ << static_cast<char>(unary_expr.type);
}

// ConstructedValue
void TypeScriptPrinter::PrintConstructedValueBegin(const ast::ConstructedValue &constr_value) {
  *out_ << "new " << constr_value.constructor_identifier.name;
  *out_ << "{";
}

void TypeScriptPrinter::PrintConstructedValueEnd() {
  *out_ << "}";
}

// Value
void TypeScriptPrinter::PrintValue(const ast::Value &value) {
  *out_ << value;
}

// VarAccess
void TypeScriptPrinter::PrintVarAccess(const ast::VarAccess &var_access) {
  *out_ << var_access;
}

// Helper getters
// Maybe it is better to move them to other file like "utils"

std::string TypeScriptPrinter::GetVariableName(const ast::TypedVariable &var) {
  return var.name.GetString();
}

std::string TypeScriptPrinter::GetVariableType(const ast::TypedVariable &var) {
  return var.type_expression.identifier.name.GetString();
}

std::string TypeScriptPrinter::GetExpressionType(const ast::TypeExpression &expr) {
  return expr.identifier.name.GetString();
}

} // namespace dbuf::gen