#include "core/codegen/sharp_target/sharp_print.h"
#include "core/ast/expression.h"

namespace dbuf::gen {

constexpr unsigned int SharpTypes::HashString(const std::string &str) {
    unsigned int hash = 5381;
    int c;

    auto str_iter = str.begin();
    while ((c = *str_iter++) != 0) {
        hash = ((hash << 5) + hash) + c;
    }

    return hash;
}

constexpr std::string SharpTypes::ConstructSharpType(const std::string &dbuf_type) {
    switch (HashString(dbuf_type)) {
        case HashString("Bool"):
            return kTypes[0];
            break;
        case HashString("Float"):
            return kTypes[1];
            break;
        case HashString("Int"):
            return kTypes[2];
            break;
        case HashString("String"):
            return kTypes[3];
            break;
        case HashString("Unsigned"):
            return kTypes[4];
            break;
    }
    std::cerr << "Unsupported sharp type: " << dbuf_type << std::endl;
    return "";
}

SharpPrinter::SharpPrinter(std::shared_ptr<std::ofstream> output) {
    out_ = std::move(output);
}

void SharpPrinter::InitFile() {
    *out_ << kReadme;
}

// Class
void SharpPrinter::PrintClassBegin(const std::string &name) {
    *out_ << "public class " << name << " {\n";
}

void SharpPrinter::PrintClassEnd() {
    *out_ << "}\n";
}

// Enum
void SharpPrinter::PrintEnumBegin(const std::string &name) {
    *out_ << "public enum " << name << " {\n";
}

void SharpPrinter::PrintEnumEnd() {
    *out_ << "}\n";
}

// TypedVariables
void SharpPrinter::PrintTypedVariables(
        const std::vector<ast::TypedVariable> &variables,
        std::string &&delimeter,
        bool add_last_delimeter,
        bool is_public,
        bool need_getter_and_setter) {
    bool first = true;
    for (const auto &var : variables) {
        if (first) {
            first = false;
        } else {
            *out_ << delimeter;
        }
        const std::string &access = is_public ? "public" : "private";
        const std::string &type = GetVariableType(var);
        const std::string &name = GetVariableName(var);
        *out_ << access << " " << type_constructor_.ConstructSharpType(type) << " " << name;
        if (need_getter_and_setter) {
            *out_ << " " << kGetterAndSetter;
        }
    }
    if (add_last_delimeter && !first) {
        *out_ << delimeter;
    }
}

// TypeExpression
void SharpPrinter::PrintTypeExpression(
        const ast::TypeExpression &expression,
        bool is_public) {
    const std::string &access = is_public ? "public" : "private";
    const std::string &type = GetExpressionType(expression);
    *out_ << access << " " << type_constructor_.ConstructSharpType(type);
}

// BinaryExpression
void SharpPrinter::PrintBinaryExpressionBegin() {
    *out_ << "(";
}

void SharpPrinter::PrintBinaryExpressionType(const ast::BinaryExpression &binary_expr) {
    *out_ << " " << static_cast<char>(binary_expr.type) << " ";
}

void SharpPrinter::PrintBinaryExpressionEnd() {
    *out_ << ")";
}

// UnaryExpression
void SharpPrinter::PrintUnaryExpressionType(const ast::UnaryExpression &unary_expr) {
    *out_ << static_cast<char>(unary_expr.type);
}

// ConstructedValue
void SharpPrinter::PrintConstructedValueBegin(const ast::ConstructedValue &constr_value) {
    *out_ << constr_value.constructor_identifier.name;
    *out_ << "{";
}

void SharpPrinter::PrintConstructedValueEnd() {
    *out_ << "}\n";
}

// Value
void SharpPrinter::PrintValue(const ast::Value &value) {
    *out_ << value;
}

// VarAccess
void SharpPrinter::PrintVarAccess(const ast::VarAccess &var_access) {
    *out_ << var_access;
}

// Helper getters
// Maybe it is better to move them to other file like "utils"

std::string SharpPrinter::GetVariableName(const ast::TypedVariable &var) {
    return var.name.GetString();
}

std::string SharpPrinter::GetVariableType(const ast::TypedVariable &var) {
    return var.type_expression.identifier.name.GetString();
}

std::string SharpPrinter::GetExpressionType(const ast::TypeExpression &expr) {
    return expr.identifier.name.GetString();;
}

} // namespace dbuf::gen