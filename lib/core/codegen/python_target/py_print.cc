#include "core/codegen/python_target/py_print.h"

#include "core/codegen/python_target/utils.h"

#include <memory>
#include <sstream>
#include <unordered_map>

namespace dbuf::gen {

PyPrinter::PyPrinter(std::shared_ptr<std::ofstream> output) {
  out_ = std::move(output);

  std::string tab = "    ";
  tabs_.emplace_back("");
  for (int i = 1; i <= 7; ++i) {
    tabs_.push_back(tabs_[i - 1] + tab);
  }
}

// public methods

void PyPrinter::init_file() {
  *out_ << kReadme << kImports << kCreateUnsigned;
  print_dbuf_error();
}

void PyPrinter::print_def_is_consistent() {
  print_line();
  print_line();
  print_line({"def _is_consistent(actual: tuple, expected: tuple) -> bool:"});
  print_line({"for i in range(len(actual)):"}, 1);
  print_line({"if expected[i] is None:"}, 2);
  print_line({"continue"}, 3);
  print_line();
  print_line({"if actual[i] != expected[i]:"}, 2);
  print_line({"return False"}, 3);
  print_line();
  print_line({"return True"}, 1);
}

void PyPrinter::print_line(const std::vector<std::string> &tokens, int level) {
  *out_ << tabs_[level];

  for (const auto &token : tokens) {
    *out_ << token;
  }
  *out_ << "\n";
}

void PyPrinter::print_line(std::vector<std::string> &&tokens, int level) {
  print_line(tokens, level);
}

void PyPrinter::print_outer_class(const std::string &name) {
  // class {name}:
  print_line();
  print_line();
  print_line({"class ", name, ":"});
}

void PyPrinter::print_inner_class(const std::string &name, int level) {
  // @dataclass
  // class __{name}:
  print_line({"@dataclass"}, level);
  print_line({"class ", "__", name, ":"}, level);
}

void PyPrinter::print_inner_class_field(const std::string &name, const std::string &type, int level) {
  // "  {name}: {py_type}"
  std::string py_type = get_python_type(type);
  print_line({name, ": ", py_type}, level);
}

void PyPrinter::print_def_check_message(
    const std::vector<std::string> &dep_names,
    const std::vector<std::string> &dep_types,
    const std::vector<std::string> &field_names,
    const std::vector<std::string> &field_deps,
    const std::vector<bool> &field_need_ignores,
    int level) {
  // def check(self, x: int, y: float, z: str) -> None:
  std::string def_name = "check";
  print_instance_method(def_name, dep_names, dep_types, level);
  level++;

  bool fields_with_deps = false;
  for (int i = 0; i < field_names.size(); ++i) {
    if (!tuple_is_empty(field_deps[i])) {
      if (fields_with_deps) {
        print_line();
      }

      fields_with_deps = true;
      print_var_deps(field_names[i], field_deps[i], field_need_ignores[i], level);
      std::vector<std::string> tokens = {"self.", field_names[i], ".check(*", field_names[i], "_deps)"};
      print_line(tokens, level);
    }
  }

  if (!fields_with_deps) {
    print_line({"pass"}, level);
  }
}

void PyPrinter::print_def_check_enum(
    const std::vector<std::string> &dep_names,
    const std::vector<std::string> &dep_types,
    const std::vector<std::string> &field_names,
    const std::vector<std::string> &field_deps,
    const std::vector<bool> &field_need_ignores,
    const std::string &struct_name,
    const std::string &constructor_name,
    int level) {
  // def check(self, x: int, y: float, z: str) -> None:
  std::string def_name = "check";
  print_instance_method(def_name, dep_names, dep_types, level);
  level++;

  // if type(self) not in SomeMessage._possible_types(x, y, z):
  std::string args                = untyped_args(dep_names);
  std::vector<std::string> tokens = {"if type(self) not in ", struct_name, "._possible_types", args, ":"};
  print_line(tokens, level);
  level++;

  print_line({"raise _DbufError("}, level++);
  std::string type_name                  = struct_name + ".__" + constructor_name;
  std::vector<std::string> error_message = {"'Type ", type_name, " does not match given dependencies.'"};
  print_line(error_message, level--);
  print_line({")"}, level--);

  for (int i = 0; i < field_names.size(); ++i) {
    if (!tuple_is_empty(field_deps[i])) {
      print_line();
      print_var_deps(field_names[i], field_deps[i], field_need_ignores[i], level);
      std::vector<std::string> tokens = {"self.", field_names[i], ".check(*", field_names[i], "_deps)"};
      print_line(tokens, level);
    }
  }
}

void PyPrinter::print_type(const std::string &struct_name, const std::vector<std::string> &inner_types, int level) {
  // some_enam_type: TypeAlias = __Nil | __Succ
  print_line();

  std::stringstream buf;
  buf << camel_to_snake(struct_name) << "_type: TypeAlias" << " = ";
  buf << "__" << inner_types[0];

  std::string sep = " | ";
  for (int i = 1; i < inner_types.size(); ++i) {
    buf << sep << "__" << inner_types[i];
  }
  print_line({buf.str()}, level);
}

void PyPrinter::print_def_possible_types(
    const std::vector<std::string> &names,
    const std::vector<std::string> &types,
    const std::vector<std::string> &expected_params_matrix,
    const std::vector<bool> &expected_need_ignores,
    const std::vector<std::vector<std::string>> &possible_types_matrix,
    int level) {
  std::string res_type = "set[type]";
  std::string def_name = "_possible_types";

  // @classmethod
  // def _possible_types(cls, x: int, y: float) -> set[type]:
  print_line();
  print_line({"@classmethod"}, level);
  print_class_method(def_name, names, types, level, res_type);
  level++;

  print_line({"actual = ", py_tuple(names)}, level);

  for (int i = 0; i < expected_params_matrix.size(); ++i) {
    print_consistency_check(expected_params_matrix[i], possible_types_matrix[i], expected_need_ignores[i], level);
  }

  print_line({"return set()"}, level);
}

void PyPrinter::print_def_init(
    const std::vector<std::string> &names,
    const std::vector<std::string> &types,
    const std::vector<std::string> &deps,
    const std::vector<bool> &dep_need_ignores,
    int level) {
  std::string def_name = "__init__";

  print_line();

  // def __init__(self, x: int, y: float) -> None:
  print_instance_method(def_name, names, types, level);
  level++;

  // x.check(*x_deps)
  for (int i = 0; i < names.size(); ++i) {
    if (!tuple_is_empty(deps[i])) {
      print_var_deps(names[i], deps[i], dep_need_ignores[i], level);
      std::vector<std::string> tokens = {names[i], ".check(*", names[i], "_deps)"};
      print_line(tokens, level);
      print_line();
    }
  }

  // self.dependencies = (x, y)
  std::string tuple = py_tuple(names);
  print_line({"self.dependencies = ", tuple}, level);
}

void PyPrinter::print_method_construct(
    const std::string &message_name,
    const std::vector<std::string> &names,
    const std::vector<std::string> &types,
    int level) {
  std::string def_name = "construct";
  print_constructor(def_name, message_name, names, types, level);
}

void PyPrinter::print_enum_constructor(
    const std::string &subclass,
    const std::vector<std::string> &names,
    const std::vector<std::string> &types,
    int level) {
  std::string def_name = camel_to_snake(subclass);
  print_constructor(def_name, subclass, names, types, level);
}

// private methods

void PyPrinter::print_def_with_typed_args(
    const std::string &name,
    const std::string &first_arg,
    const std::vector<std::string> &arg_names,
    const std::vector<std::string> &arg_types,
    int level,
    const std::string &res_type) {
  std::string args = typed_args(arg_names, arg_types, first_arg);

  std::vector<std::string> tokens = {"def ", name, args, " -> ", res_type, ":"};
  print_line(tokens, level);
}

void PyPrinter::print_instance_method(
    const std::string &name,
    const std::vector<std::string> &arg_names,
    const std::vector<std::string> &arg_types,
    int level,
    const std::string &res_type) {
  std::string first_arg = "self";
  print_def_with_typed_args(name, first_arg, arg_names, arg_types, level, res_type);
}

void PyPrinter::print_instance_method(
    const std::string &name,
    const std::vector<std::string> &arg_names,
    const std::vector<std::string> &arg_types,
    int level) {
  std::string first_arg = "self";
  std::string res_type  = "None";
  print_def_with_typed_args(name, first_arg, arg_names, arg_types, level, res_type);
}

void PyPrinter::print_class_method(
    const std::string &name,
    const std::vector<std::string> &arg_names,
    const std::vector<std::string> &arg_types,
    int level,
    const std::string &res_type) {
  std::string first_arg = "cls";
  print_def_with_typed_args(name, first_arg, arg_names, arg_types, level, res_type);
}

void PyPrinter::print_class_method(
    const std::string &name,
    const std::vector<std::string> &arg_names,
    const std::vector<std::string> &arg_types,
    int level) {
  std::string first_arg = "cls";
  std::string res_type  = "None";
  print_def_with_typed_args(name, first_arg, arg_names, arg_types, level, res_type);
}

void PyPrinter::print_constructor(
    const std::string &def_name,
    const std::string &struct_name,
    const std::vector<std::string> &names,
    const std::vector<std::string> &types,
    int level) {
  print_line();

  // def construct(self, x: int, y: float) -> __Address:
  std::string res_type = get_python_type(struct_name, true);
  print_instance_method(def_name, names, types, level, res_type);
  level++;

  // obj = self.__Address(x, y)
  std::string args = untyped_args(names);
  print_line({"obj = self.", res_type, args}, level);

  print_line({"obj.check(*self.dependencies)"}, level);
  print_line({"return obj"}, level);
}

void PyPrinter::print_var_deps(const std::string &dep_name, const std::string &deps, bool need_ignores, int level) {
  // __k_deps = ()
  std::vector<std::string> tokens = {dep_name, "_deps = ", deps};
  if (need_ignores) {
    tokens.emplace_back("  # type: ignore[attr-defined]");
  }

  print_line(tokens, level);
}

void PyPrinter::print_consistency_check(const std::string &expected, const std::vector<std::string> &types, bool need_ignores, int level) {
  std::vector<std::string> return_set = {"return {"};
  std::string sep                     = ", ";
  for (int i = 0; i < types.size(); ++i) {
    if (i != 0) {
      return_set.push_back(sep);
    }
    return_set.emplace_back("cls.__");
    return_set.push_back(types[i]);
  }
  return_set.emplace_back("}");

  if (need_ignores) {
    print_line({"expected = ", expected, "  # type: ignore[attr-defined, assignment]"}, level);
  } else {
    print_line({"expected = ", expected}, level);
  }
  
  print_line({"if _is_consistent(actual, expected):"}, level++);
  print_line(return_set, level);
  print_line();
}

void PyPrinter::print_dbuf_error() {
  print_line();
  print_line();
  print_line({"class _DbufError(TypeError):"});
  print_line({"pass"}, 1);
}

const std::string PyPrinter::kReadme = "# This file was automatically generated by DependoBuf.\n"
                                       "# Please do not change it.\n";

const std::string PyPrinter::kImports = "\n"
                                        "from __future__ import annotations\n"
                                        "\n"
                                        "from annotated_types import Ge\n"
                                        "from dataclasses import dataclass\n"
                                        "from typing import Annotated, TypeAlias\n";

const std::string PyPrinter::kCreateUnsigned = "\n"
                                               "Unsigned = Annotated[int, Ge(0)]\n";

} // namespace dbuf::gen