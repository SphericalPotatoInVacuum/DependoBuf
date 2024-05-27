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
  py_exression_.set_constructor_type_map(tree->constructor_to_type);

  for (const auto &struct_name : tree->visit_order) {
    std::variant<ast::Message, ast::Enum> &var = tree->types[struct_name];
    std::visit(*this, var);
  }

  printer_.print_def_is_consistent();
}

// print message
void PyCodeGenerator::operator()(const ast::Message &ast_message) {
  const std::string &message_name = ast_message.identifier.name.GetString();
  printer_.print_outer_class(message_name);
  printer_.print_inner_class(message_name);

  std::vector<std::string> dep_names;
  std::vector<std::string> dep_types;
  std::vector<std::string> dep_deps;
  std::vector<bool> dep_need_ignores;
  prepare_names_types_deps_for_deps(ast_message.type_dependencies, dep_names, dep_types, dep_deps, dep_need_ignores);

  std::vector<std::string> field_names;
  std::vector<std::string> field_types;
  std::vector<std::string> field_deps;
  std::vector<bool> field_need_ignores;
  prepare_names_types_deps_for_fields(ast_message.fields, field_names, field_types, field_deps, field_need_ignores);

  if (!field_names.empty()) {
    printer_.print_line();
  }
  printer_.print_def_check_message(dep_names, dep_types, field_names, field_deps, field_need_ignores);

  std::vector<std::string> inner_types = {message_name};
  printer_.print_type(message_name, inner_types);

  printer_.print_def_init(dep_names, dep_types, dep_deps, dep_need_ignores);
  printer_.print_method_construct(message_name, field_names, field_types);
}

// print enum
void PyCodeGenerator::operator()(const ast::Enum &ast_enum) {
  const std::string &enum_name = ast_enum.identifier.name.GetString();
  printer_.print_outer_class(enum_name);

  std::vector<std::string> dep_names;
  std::vector<std::string> dep_types;
  std::vector<std::string> dep_deps;
  std::vector<bool> dep_need_ignores;
  prepare_names_types_deps_for_deps(ast_enum.type_dependencies, dep_names, dep_types, dep_deps, dep_need_ignores);

  std::vector<std::string> constructors;
  std::unordered_map<std::string, std::vector<std::string>> field_names_map;
  std::unordered_map<std::string, std::vector<std::string>> field_types_map;

  std::vector<std::string> expected_params_matrix;
  std::vector<bool> expected_need_ignores;
  std::vector<std::vector<std::string>> possible_types_matrix;

  expected_params_matrix.reserve(ast_enum.pattern_mapping.size());
  expected_need_ignores.reserve(ast_enum.pattern_mapping.size());
  possible_types_matrix.reserve(ast_enum.pattern_mapping.size());

  bool first_inner_class = true;
  for (const auto &rule : ast_enum.pattern_mapping) {
    std::string expected_params = py_exression_.get_instances(rule.inputs);
    bool need_ignores           = py_exression_.does_need_ignores();
    std::vector<std::string> possible_types;
    possible_types.reserve(rule.outputs.size());

    for (const auto &enum_constructor : rule.outputs) {
      const std::string &constructor_name = enum_constructor.identifier.name.GetString();
      possible_types.push_back(constructor_name);
      constructors.push_back(constructor_name);

      if (first_inner_class) {
        first_inner_class = false;
      } else {
        printer_.print_line();
      }
      printer_.print_inner_class(constructor_name);

      std::vector<std::string> field_names;
      std::vector<std::string> field_types;
      std::vector<std::string> field_deps;
      std::vector<bool> field_need_ignores;
      prepare_names_types_deps_for_fields(
          enum_constructor.fields,
          field_names,
          field_types,
          field_deps,
          field_need_ignores);

      if (!field_names.empty()) {
        printer_.print_line();
      }
      printer_.print_def_check_enum(
          dep_names,
          dep_types,
          field_names,
          field_deps,
          field_need_ignores,
          enum_name,
          constructor_name);

      field_names_map[constructor_name] = std::move(field_names);
      field_types_map[constructor_name] = std::move(field_types);
    }

    expected_params_matrix.push_back(std::move(expected_params));
    expected_need_ignores.push_back(need_ignores);
    possible_types_matrix.push_back(std::move(possible_types));
  }

  printer_.print_type(enum_name, constructors);

  printer_.print_def_possible_types(
      dep_names,
      dep_types,
      expected_params_matrix,
      expected_need_ignores,
      possible_types_matrix);

  printer_.print_def_init(dep_names, dep_types, dep_deps, dep_need_ignores);

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

void PyCodeGenerator::prepare_names_types_deps_for_deps(
    const std::vector<ast::TypedVariable> &typed_vars,
    std::vector<std::string> &names,
    std::vector<std::string> &types,
    std::vector<std::string> &deps,
    std::vector<bool> &need_ignores) {
  size_t dep_size = typed_vars.size();
  names           = std::vector<std::string>(dep_size);
  types           = std::vector<std::string>(dep_size);
  deps            = std::vector<std::string>(dep_size);
  need_ignores    = std::vector<bool>(dep_size);

  py_exression_.clear_field_names_kit();
  for (size_t i = 0; i < dep_size; ++i) {
    auto [name, type] = get_name_and_type(typed_vars[i]);
    names[i]          = name;
    types[i]          = type;

    std::string var_deps = py_exression_.get_instances(typed_vars[i].type_expression.parameters);
    deps[i]              = std::move(var_deps);
    need_ignores[i]      = py_exression_.does_need_ignores();
  }
}

void PyCodeGenerator::prepare_names_types_deps_for_fields(
    const std::vector<ast::TypedVariable> &typed_vars,
    std::vector<std::string> &names,
    std::vector<std::string> &types,
    std::vector<std::string> &deps,
    std::vector<bool> &need_ignores) {
  size_t dep_size = typed_vars.size();
  names           = std::vector<std::string>(dep_size);
  types           = std::vector<std::string>(dep_size);
  deps            = std::vector<std::string>(dep_size);
  need_ignores    = std::vector<bool>(dep_size);

  for (size_t i = 0; i < dep_size; ++i) {
    auto [name, type] = get_name_and_type(typed_vars[i]);
    printer_.print_inner_class_field(name, type);

    names[i] = name;
    types[i] = type;
  }

  py_exression_.set_field_names_kit(names);
  for (size_t i = 0; i < dep_size; ++i) {
    std::string var_deps = py_exression_.get_instances(typed_vars[i].type_expression.parameters);
    deps[i]              = std::move(var_deps);
    need_ignores[i]      = py_exression_.does_need_ignores();
  }
}

} // namespace dbuf::gen