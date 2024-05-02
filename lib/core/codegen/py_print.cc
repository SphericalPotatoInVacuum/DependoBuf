#include "core/codegen/py_print.h"

#include <iostream>
#include <memory>
#include <unordered_map>
#include <vector>

namespace dbuf::gen {
const std::unordered_map<std::string, std::string> PyPrinter::kBuildInTypes = {
    {"Int", "int"},
    {"Unsigned", "Unsigned"},
    {"Float", "float"},
    {"String", "str"},
    {"Bool", "bool"},
};

const std::string PyPrinter::kReadme = "# This file was automatically generated by DependoBuf.\n"
                                       "# Please do not change it.\n";

const std::string PyPrinter::kImports = "\n"
                                        "from annotated_types import Ge\n"
                                        "from dataclasses import dataclass\n"
                                        "from typing import Annotated\n";

const std::string PyPrinter::kCreateUnsigned = "\n"
                                               "Unsigned = Annotated[int, Ge(0)]\n";

PyPrinter::PyPrinter(std::shared_ptr<std::ofstream> output) {
  out_ = std::move(output);

  std::string tab = "    ";
  kTab.push_back("");
  for (int i = 1; i <= 4; ++i) {
    kTab.push_back(kTab[i - 1] + tab);
  }
}

std::string PyPrinter::get_python_type(const std::string &type) {
  if (kBuildInTypes.contains(type)) {
    return kBuildInTypes.at(type);
  }

  std::string struct_type = type + "." + camel_to_snake(type) + "_type";
  return struct_type;
}

std::string PyPrinter::camel_to_snake(const std::string &camel_str) {
  std::string snake_str = "";
  for (int i = 0; i < camel_str.size(); ++i) {
    char c = camel_str[i];

    if ('A' <= c && c <= 'Z') {
      c = (char)tolower(c);
      if (i != 0) {
        snake_str += '_';
      }
    }

    snake_str += c;
  }

  return snake_str;
}

void PyPrinter::print_line(std::vector<std::string> tokens = {}, int level = 0) {
  *out_ << kTab[level];

  for (auto &token : tokens) {
    *out_ << token;
  }
  *out_ << "\n";
}

void PyPrinter::init_file() {
  *out_ << kReadme << kImports << kCreateUnsigned;
}

void PyPrinter::print_outer_class(const std::string &name) {
  // class {name}:
  print_line();
  print_line();
  print_line({"class ", name, ":"});
}

void PyPrinter::print_inner_class(const std::string &name) {
  // @dataclass
  // class __{name}:
  print_line({"@dataclass"}, 1);
  print_line({"class ", "__", name}, 1);
}

void PyPrinter::print_type(const std::string &struct_name, std::vector<std::string> &inner_types, int level = 1) {
  // some_enam_type = __Nil | __Succ
  std::string left = camel_to_snake(struct_name) + "_type";
  std::string right = inner_types[0];

  std::string sep = " | ";
  for (int i = 1; i < inner_types.size(); ++i) {
    right += sep + "__" + inner_types[i];
  }
  print_line({left, " = ", right}, level);
}

void PyPrinter::print_def_check(
    std::vector<std::string> &names,
    std::vector<std::string> &types,
    std::unordered_map<std::string, std::vector<std::string>> &fields_deps,
    const std::string &struct_name,
    int level = 2) {
  std::vector<std::string> tokens;
  tokens.push_back("def check(self");

  std::string sep = ", ";
  for (int i = 0; i < names.size(); ++i) {
    std::string py_type = get_python_type(types[i]);
    tokens.push_back(sep + names[i] + ": " + py_type);
  }
  tokens.push_back(") -> None:");

  print_line(tokens, level);
  level++;

  tokens.clear();
  tokens.push_back("if type(self) not in ");
  tokens.push_back(struct_name + ".possible_types(");
  for (int i = 0; i < names.size(); ++i) {
    if (i != 0) {
      tokens.push_back(sep);
    }

    tokens.push_back(names[i]);
  }
  tokens.push_back("):");

  print_line(tokens, level);
  level++;

  std::string raise_exc = "raise TypeError('Non-compliance with type dependencies')";
  print_line({raise_exc}, level);
  level--;
}

void PyPrinter::print_def_init(
    std::vector<std::string> &names,
    std::vector<std::string> &types,
    std::vector<std::vector<std::string>> &deps,
    int level = 1) {
  std::vector<std::string> tokens;
  tokens.push_back("def __init__(self");

  std::string sep = ", ";
  for (int i = 0; i < names.size(); ++i) {
    std::string py_type = get_python_type(types[i]);
    tokens.push_back(sep + names[i] + ": " + py_type);
  }
  tokens.push_back(") -> None:");
  print_line(tokens, level);
  level++;

  for (auto &name : names) {
    // {x}.check(*self.__{x}_deps)
    std::string line = name + ".check(*self.__" + name + "deps)";
    print_line({line}, level);

    // self.x = x
    std::string line = "self." + name + " = " + name;
    print_line({line}, level);

    print_line();
  }
}

void PyPrinter::print_field(const std::string &name, const std::string &type, int level) {
  // "  {name}: {py_type}"
  std::string py_type = get_python_type(type);
  print_line({name, ": ", type}, level);
}

} // namespace dbuf::gen