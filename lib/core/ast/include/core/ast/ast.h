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
  uint64_t name;
  TypeExpression type_expression;
};

struct Constructor {
  uint64_t name;
  std::vector<TypedVariable> fields = {};
};

struct Message {
  uint64_t name;
  std::vector<TypedVariable> type_dependencies = {};
  std::vector<TypedVariable> fields            = {};
};

struct Enum {
  struct Rule {
    using InputPattern = std::variant<Value, Star>;

    std::vector<InputPattern> inputs = {};
    std::vector<Constructor> outputs = {};
  };

  uint64_t name;
  std::vector<TypedVariable> type_dependencies = {};
  std::vector<Rule> pattern_mapping            = {};
};

struct AST {
  void AddMessage(Message &&message_arg);
  void AddEnum(Enum &&enum_arg);

  uint64_t GetInterning(std::string &&input_string);

  struct {
    uint64_t GetInterning(std::string &&input_string) {
      auto result = tokens.try_emplace(std::move(input_string), counter);
      if (result.second) {
        counter++;
      }
      return result.first->second;
    }

    std::unordered_map<std::string, uint64_t> tokens;
    uint64_t counter {0};
  } interning = {};

  std::unordered_map<uint64_t, Message> messages;
  std::unordered_map<uint64_t, Enum> enums;
};

} // namespace dbuf::ast
