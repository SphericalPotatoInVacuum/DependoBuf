#include "core/codegen/sharp_target/sharp_gen.h"
#include "core/ast/ast.h"
#include "core/codegen/sharp_target/sharp_print.h"
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
    printer_.PrintStructBegin(message_name);

    std::vector<ast::TypedVariable> variables;
    variables.reserve(ast_message.fields.size());

    for (const auto &field : ast_message.fields) {
        variables.push_back(field);
    }
    printer_.PrintVariables(variables, ";\n", true);
    printer_.PrintStructEnd();
}

void SharpCodeGenerator::operator()(const ast::Enum &ast_enum) {

}

} // namespace dbuf::gen