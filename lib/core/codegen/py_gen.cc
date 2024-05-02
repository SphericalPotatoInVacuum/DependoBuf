#include "core/codegen/py_gen.h"

#include "core/ast/ast.h"

#include <variant>

namespace dbuf::gen {

PyCodeGenerator::PyCodeGenerator(const std::string &out_file)
    : ITargetCodeGenerator(out_file) {
  printer_ = PyPrinter(output_);
}

void PyCodeGenerator::Generate(ast::AST *tree) {
  if (tree == nullptr) {
    std::cerr << "Python codegen did\'t get AST" << std::endl;
    return;
  }

  printer_.init_file();

  for (const auto &struct_name : tree->visit_order) {
    std::variant<ast::Message, ast::Enum> &var = tree->types[struct_name];
    if (std::holds_alternative<ast::Message>(var)) {
      auto &mes = std::get<ast::Message>(var);
      (*this)(mes);
    }
  }
}

void PyCodeGenerator::operator()(const ast::Message &ast_message) {
  const std::string &message_name = ast_message.identifier.name.GetString();
  printer_.print_outer_class(message_name);
  printer_.print_inner_class(message_name);

  
  std::unordered_map<std::string, std::vector<std::string>> fields_deps;
  for (const auto &field : ast_message.fields) {
    (*this)(field);
  }

  std::vector<std::string> names;
  std::vector<std::string> types;
  for (const auto &dep : ast_message.type_dependencies) {
    auto [name, type] = get_name_and_type(dep);
    names.push_back(name);
    names.push_back(type);
  }
  printer_.print_def_check(names, types, fields_deps, message_name);
}

void PyCodeGenerator::operator()(const ast::Enum &ast_enum) {}

void PyCodeGenerator::operator()(const ast::TypedVariable &typed_var) {
  const std::string &field_name = typed_var.name.GetString();
  const std::string &field_type = typed_var.type_expression.identifier.name.GetString();
  printer_.print_field(field_name, field_type, 2);
}

std::tuple<std::string, std::string> get_name_and_type(const ast::TypedVariable &var) {
  std::string name = var.name.GetString();
  std::string type = var.type_expression.identifier.name.GetString();
  return make_tuple(name, type);
}

} // namespace dbuf::gen
