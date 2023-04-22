#pragma once

#include "core/ast/expression.h"

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace dbuf::ast {

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

class AST {
public:
  void AddMessage(Message &&message_arg);
  void AddEnum(Enum &&enum_arg);

  uint64_t GetInterning(std::string &&input_string);

private:
  struct {
    uint64_t GetInterning(std::string &&input_string) {
      auto result = tokens_.try_emplace(std::move(input_string), counter_);
      if (result.second) {
        counter_++;
      }
      return result.first->second;
    }

    std::unordered_map<std::string, uint64_t> tokens_;
    uint64_t counter_ {0};
  } interning_ = {};

  std::unordered_map<uint64_t, Message> messages_;
  std::unordered_map<uint64_t, Enum> enums_;
};

} // namespace dbuf::ast
