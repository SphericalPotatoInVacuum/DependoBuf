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

  for (auto &field: ast_message.fields) {
    auto [name, type] = get_name_and_type(field);
    printer_.print_inner_class_field(name, type);
  }

  std::unordered_map<std::string, std::vector<std::string>> fields_deps;
  std::vector<std::string> dep_names;
  std::vector<std::string> dep_types;
  for (const auto &dep : ast_message.type_dependencies) {
    auto [name, type] = get_name_and_type(dep);
    dep_names.push_back(name);
    dep_types.push_back(type);
  }
  printer_.print_def_check(dep_names, dep_types, fields_deps, message_name);

  std::vector<std::string> inner_types = {message_name};
  printer_.print_type(message_name, inner_types);
  for (auto &dep_name: dep_names) {
    std::vector<std::string> deps = {};
    printer_.print_dep_deps(dep_name, deps);
  }

  printer_.print_def_init(dep_names, dep_types);
}

void PyCodeGenerator::operator()(const ast::Enum &ast_enum) {}

void PyCodeGenerator::operator()(const ast::TypedVariable &typed_var) {
  const std::string &field_name = typed_var.name.GetString();
  const std::string &field_type = typed_var.type_expression.identifier.name.GetString();
  printer_.print_inner_class_field(field_name, field_type);
}

std::tuple<std::string, std::string> PyCodeGenerator::get_name_and_type(const ast::TypedVariable &var) {
  std::string name = var.name.GetString();
  std::string type = var.type_expression.identifier.name.GetString();
  return make_tuple(name, type);
}

} // namespace dbuf::gen
