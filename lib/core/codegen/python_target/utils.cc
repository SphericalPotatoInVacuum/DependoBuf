#include "core/codegen/python_target/utils.h"

#include <sstream>

namespace dbuf::gen {

std::string camel_to_snake(const std::string &camel_str) {
  std::string snake_str;
  for (int i = 0; i < camel_str.size(); ++i) {
    char c = camel_str[i];

    if ('A' <= c && c <= 'Z') {
      c = static_cast<char>(tolower(c));
      if (i != 0) {
        snake_str += '_';
      }
    }

    snake_str += c;
  }

  return snake_str;
}

std::string get_python_type(const std::string &type, bool inside_outer_class) {
  if (BUILD_IN_TYPES.contains(type)) {
    return BUILD_IN_TYPES.at(type);
  }

  if (inside_outer_class) {
    return "__" + type;
  }

  std::string struct_type = type + "." + camel_to_snake(type) + "_type";
  return struct_type;
}

std::string typed_args(
    const std::vector<std::string> &names,
    const std::vector<std::string> &types,
    const std::string &first_arg) {
  // (self, x: int, y: float)
  std::stringstream args;
  args << "(" << first_arg;
  std::string sep = ", ";

  for (int i = 0; i < names.size(); ++i) {
    std::string py_type = get_python_type(types[i]);
    args << sep << names[i] << ": " << py_type;
  }
  args << ")";
  return args.str();
}

std::string untyped_args(const std::vector<std::string> &names) {
  // (x, y, z)
  std::string args = "(";
  std::string sep  = ", ";

  for (int i = 0; i < names.size(); ++i) {
    if (i != 0) {
      args += sep;
    }
    args += names[i];
  }

  args += ")";
  return args;
}

std::string py_tuple(const std::vector<std::string> &names) {
  // (x, y, )
  std::string tup = "(";
  std::string sep = ", ";

  for (int i = 0; i < names.size(); ++i) {
    tup += names[i];
    tup += sep;
  }
  tup += ")";
  return tup;
}

bool tuple_is_empty(const std::string &tuple_str) {
  return (tuple_str.size() <= 2);
}

}