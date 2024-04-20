#include "core/codegen/py_gen.h"
#include "core/ast/ast.h"
#include <variant>

namespace dbuf::gen {
void print_class(std::string &name) {
  
}

void PyCodeGenerator::Generate(ast::AST *tree) {
  if (tree == nullptr) {
    std::cerr << "Python codegen did\'t get AST" << std::endl;
    return;
  }

  for (const auto &struct_name: tree->visit_order) {
    std::variant<ast::Message, ast::Enum> &var = tree->types[struct_name];
    std::visit(*this, var);
    if (true) {

    }
  }
}

void PyCodeGenerator::operator()(const ast::Message &ast_message) {
  *output_ << "class " << ast_message.identifier.name.GetString() << ":" << std::endl;
  for (const auto &field: ast_message.fields) {
    (*this)(field);
  }
}

} // namespace dbuf::gen
