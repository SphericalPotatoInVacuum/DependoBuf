#include "core/codegen/python_target/py_gen.h"

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
    } else {
      auto &enu = std::get<ast::Enum>(var);
      (*this)(enu);
    }
  }
}

// print message
void PyCodeGenerator::operator()(const ast::Message &ast_message) {
  const std::string &message_name = ast_message.identifier.name.GetString();
  printer_.print_outer_class(message_name);

  std::vector<std::string> dep_names;
  std::vector<std::string> dep_types;
  for (const auto &dep : ast_message.type_dependencies) {
    auto [name, type] = get_name_and_type(dep);
    dep_names.push_back(name);
    dep_types.push_back(type);
  }

  printer_.print_inner_class(message_name);

  std::vector<std::string> field_names;
  std::vector<std::string> field_types;
  for (const auto &field : ast_message.fields) {
    auto [name, type] = get_name_and_type(field);
    field_names.push_back(name);
    field_types.push_back(type);
    printer_.print_inner_class_field(name, type);
  }

  printer_.print_def_check(dep_names, dep_types, message_name);

  std::vector<std::string> inner_types = {message_name};
  printer_.print_type(message_name, inner_types);
  for (auto &dep_name : dep_names) {
    printer_.print_dep_deps(dep_name);
  }

  printer_.print_def_possible_types(dep_names, dep_types);
  printer_.print_def_init(dep_names, dep_types);
  printer_.print_method_construct(message_name, field_names, field_types);
}

// print enum
void PyCodeGenerator::operator()(const ast::Enum &ast_enum) {
  const std::string &enum_name = ast_enum.identifier.name.GetString();
  printer_.print_outer_class(enum_name);

  std::vector<std::string> dep_names;
  std::vector<std::string> dep_types;
  for (const auto &dep : ast_enum.type_dependencies) {
    auto [name, type] = get_name_and_type(dep);
    dep_names.push_back(name);
    dep_types.push_back(type);
  }

  std::vector<std::string> constructors;
  std::unordered_map<std::string, std::vector<std::string>> field_names_map;
  std::unordered_map<std::string, std::vector<std::string>> field_types_map;

  for (const auto &rule : ast_enum.pattern_mapping) {
    for (const auto &enum_constructor : rule.outputs) {
      const std::string &constructor_name = enum_constructor.identifier.name.GetString();
      constructors.push_back(constructor_name);
      printer_.print_inner_class(constructor_name);

      std::vector<std::string> field_names;
      std::vector<std::string> field_types;
      for (const auto &field : enum_constructor.fields) {
        auto [name, type] = get_name_and_type(field);
        field_names.push_back(name);
        field_types.push_back(type);

        printer_.print_inner_class_field(name, type);
      }

      field_names_map[constructor_name] = std::move(field_names);
      field_types_map[constructor_name] = std::move(field_types);

      printer_.print_def_check(dep_names, dep_types, enum_name);
    }
  }

  printer_.print_type(enum_name, constructors);

  for (auto &dep_name : dep_names) {
    printer_.print_dep_deps(dep_name);
  }

  printer_.print_def_possible_types(dep_names, dep_types);
  printer_.print_def_init(dep_names, dep_types);

  for (auto &constructor_name : constructors) {
    printer_.print_enum_constructor(
        constructor_name,
        field_names_map[constructor_name],
        field_types_map[constructor_name]);
  }
}

std::tuple<std::string, std::string> PyCodeGenerator::get_name_and_type(const ast::TypedVariable &var) {
  std::string name = var.name.GetString();
  std::string type = var.type_expression.identifier.name.GetString();
  return make_tuple(name, type);
}

} // namespace dbuf::gen