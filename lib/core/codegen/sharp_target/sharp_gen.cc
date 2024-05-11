#include "core/codegen/sharp_target/sharp_gen.h"
#include "core/ast/ast.h"
#include "core/ast/expression.h"
#include "core/codegen/sharp_target/sharp_print.h"
#include <iostream>
#include <variant>
#include <vector>

namespace dbuf::gen {

SharpCodeGenerator::SharpCodeGenerator(const std::string &out_file)
        : ITargetCodeGenerator(out_file) {
    printer_ = SharpPrinter(output_);
};

void SharpCodeGenerator::Generate(const ast::AST *tree) {
    if (tree == nullptr) {
        std::cerr << "Sharp codegen did not get AST" << std::endl;
    } else {
        std::cerr << "in Generate" << std::endl;
        tree_ = tree;

        for (const auto &type: tree->visit_order) {
            const std::variant<ast::Message, ast::Enum> &var = tree->types.at(type);
            if (std::holds_alternative<ast::Message>(var)) {
                const auto &dbuf_message = std::get<ast::Message>(var);
                (*this)(dbuf_message);
            } else {
                const auto &dbuf_enum = std::get<ast::Enum>(var);
                (*this)(dbuf_enum);
            }
        }
    }
}

void SharpCodeGenerator::operator()(const ast::Message &ast_message) {
    const std::string &message_name = ast_message.identifier.name.GetString();
    std::cout << "Call message generaton: " << message_name << std::endl;
    printer_.PrintClassBegin(message_name);

    // std::vector<ast::TypedVariable> variables;
    // variables.reserve(ast_message.fields.size());

    for (const auto &field : ast_message.fields) {
        (*this)(field);
    }
    if (!ast_message.type_dependencies.empty()) {
        *output_ << "template <";
        printer_.PrintTypedVariables(ast_message.type_dependencies, ", ", true, true);
        *output_ << ">\n";
    }
    printer_.PrintClassEnd();
}

void SharpCodeGenerator::operator()(const ast::Enum &ast_enum) {
    const std::string &enum_name = ast_enum.identifier.name.GetString();
    std::cout << "Call enum generaton: " << enum_name << std::endl;
    // printer_.PrintEnumBegin(message_name);
    // printer_.PrintEnumEnd();
}

void SharpCodeGenerator::operator()(const ast::TypedVariable &variable) {
    std::cout << "Call operator TypedVariable for " << variable.name << std::endl; 
    (*this)(variable.type_expression);
    *output_ << " " << variable.name << "\n";
}

void SharpCodeGenerator::operator()(const ast::Expression &expr) {
    if (std::holds_alternative<ast::TypeExpression>(expr)) {
        const auto &type_expr = std::get<ast::TypeExpression>(expr);
       (*this)(type_expr);
    } else if (std::holds_alternative<ast::BinaryExpression>(expr)) {
        const auto &binary_expr = std::get<ast::BinaryExpression>(expr);
       (*this)(binary_expr);
    } else if (std::holds_alternative<ast::UnaryExpression>(expr)) {
        const auto &unary_expr = std::get<ast::UnaryExpression>(expr);
       (*this)(unary_expr);
    } else if (std::holds_alternative<ast::Value>(expr)) {
        const auto &val = std::get<ast::Value>(expr);
       (*this)(val);
    } else if (std::holds_alternative<ast::VarAccess>(expr)) {
        const auto &var_access = std::get<ast::VarAccess>(expr);
       (*this)(var_access);
    } else {
        std::cerr << "Unknown expression type in code generation" << std::endl;
    }
}

void SharpCodeGenerator::operator()(const ast::TypeExpression &expr) {
    std::cout << "Call operator TypeExpression for " << expr.identifier.name << std::endl; 
    if (IsSimpleType(expr.identifier.name)) {
        printer_.PrintTypeExpression(expr, true);
    } else {
        const std::variant<ast::Message, ast::Enum> &var = tree_->types.at(expr.identifier.name);
        *output_ << expr.identifier.name;
        for (size_t ind = 0; ind < expr.parameters.size(); ++ind) {
            if (ind == 0) {
                *output_ << "<";
            }
            std::cerr << "Need to call visit in complicated TypeExpression for expr.parameters[ind]" << std::endl;
            (*this)(*expr.parameters[ind]);
            if (ind == expr.parameters.size() - 1) {
                *output_ << ">";
            } else {
                *output_ << ", ";
            }
        }
    }
}

void SharpCodeGenerator::operator()(const ast::BinaryExpression &expr) {
    std::cout << "Call operator BinaryExpression for " << static_cast<char>(expr.type) << std::endl;
    printer_.PrintBinaryExpressionBegin();
    (*this)(*expr.left);
    printer_.PrintBinaryExpressionType(expr);
    (*this)(*expr.right);
    printer_.PrintBinaryExpressionEnd();
}

void SharpCodeGenerator::operator()(const ast::UnaryExpression &expr) {
    std::cout << "Call operator UnaryExpression for " << static_cast<char>(expr.type) << std::endl;
    printer_.PrintUnaryExpressionType(expr);
    (*this)(*expr.expression);
}

void SharpCodeGenerator::operator()(const ast::ConstructedValue &value) {
    std::cout << "Call operator ConstructedValue for " << value.constructor_identifier.name << std::endl;
    printer_.PrintConstructedValueBegin(value);
    for (size_t ind = 0; ind < value.fields.size(); ++ind) {
        (*this)(*value.fields[ind].second);
        if (ind != value.fields.size() - 1) {
            *output_ << ", ";
        }
    }
    printer_.PrintConstructedValueEnd();
}

void SharpCodeGenerator::operator()(const ast::Value &value) {
    if (std::holds_alternative<ast::ConstructedValue>(value)) {
        std::cout << "Call operator Value for ConstructedValue" << std::endl;
        const auto &constructed_value = std::get<ast::ConstructedValue>(value);
        (*this)(constructed_value);
    } else {
        std::cout << "Call operator Value for " << value << std::endl;
        printer_.PrintValue(value);
    }
}

void SharpCodeGenerator::operator()(const ast::VarAccess &var_access) {
    std::cout << "Call operator VarAccess for " << var_access << std::endl;
    printer_.PrintVarAccess(var_access);
}

// Helper function. Maybe need to be moved to other file like "utils"

bool SharpCodeGenerator::IsSimpleType(const InternedString &interned_string) {
    return interned_string == InternedString("Int") ||
           interned_string == InternedString("Unsigned") ||
           interned_string == InternedString("String") ||
           interned_string == InternedString("Float") ||
           interned_string == InternedString("Bool");
}

} // namespace dbuf::gen