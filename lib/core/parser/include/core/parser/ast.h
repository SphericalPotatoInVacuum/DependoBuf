#pragma once

#include "core/parser/expression.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace dbuf::parser {

struct TypedVariable {
  uint64_t name_;
  TypeExpression type_expression_;
};

struct Constructor {
  uint64_t name_;
  std::vector<TypedVariable> fields_ = {};
};

struct Message {
  uint64_t name_;
  std::vector<TypedVariable> type_dependencies_ = {};
  std::vector<TypedVariable> fields_            = {};
};

struct Enum {
  uint64_t name_;
  std::vector<TypedVariable> type_dependencies_                    = {};
  std::vector<std::vector<std::variant<Value, StarValue>>> inputs_ = {};
  std::vector<std::vector<Constructor>> outputs_                   = {};
};

struct AST {
  void AddMessage(Message &&message_arg);
  void AddEnum(Enum &&enum_arg);

  std::unordered_map<uint64_t, Message> messages_;
  std::unordered_map<uint64_t, Enum> enums_;
};

} // namespace dbuf::parser
