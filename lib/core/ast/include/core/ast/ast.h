#pragma once

#include "core/ast/expression.h"
#include "core/interning/interned_string.h"

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace dbuf::ast {

struct TypedVariable {
  InternedString name;
  TypeExpression type_expression;
};

struct Constructor {
  InternedString name;
  std::vector<TypedVariable> fields = {};
};

struct Message {
  InternedString name;
  std::vector<TypedVariable> type_dependencies = {};
  std::vector<TypedVariable> fields            = {};
};

struct Enum {
  struct Rule {
    using InputPattern = std::variant<Value, Star>;

    std::vector<InputPattern> inputs = {};
    std::vector<Constructor> outputs = {};
  };

  InternedString name;
  std::vector<TypedVariable> type_dependencies = {};
  std::vector<Rule> pattern_mapping            = {};
};

struct AST {
  std::unordered_map<InternedString, Message> messages;
  std::unordered_map<InternedString, Enum> enums;
};

} // namespace dbuf::ast
