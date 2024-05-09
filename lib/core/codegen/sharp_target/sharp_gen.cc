#include "core/codegen/sharp_target/sharp_gen.h"

namespace dbuf::gen {

void SharpCodeGenerator::Generate(const ast::AST *tree) {
    if (tree == nullptr) {
        std::cerr << "Sharp codegen did not get AST" << std::endl;
    } else {
        tree_ = tree;
        std::cout << "Sharp codegen was not emplemented yet" << std::endl;
        
        // for (const auto &type: tree->visit_order) {
        //     const std::variant<ast::Message, ast::Enum>& var = tree->types.at(type);
        //     std::visit(*this, var);
        // }
    }
}

void SharpCodeGenerator::operator()(const ast::Message &ast_message) {
    
}

void SharpCodeGenerator::operator()(const ast::Enum &ast_enum) {

}

} // namespace dbuf::gen