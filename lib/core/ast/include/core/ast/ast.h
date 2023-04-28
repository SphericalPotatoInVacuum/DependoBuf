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

struct TypedVariable : NamedType {
  TypeExpression type_expression;
};

struct DependentType {
  std::vector<TypedVariable> type_dependencies = {};
};

struct TypeWithFields {
  std::vector<TypedVariable> fields = {};
};

struct Constructor
    : NamedType
    , TypeWithFields {};

struct Message
    : NamedType
    , DependentType
    , TypeWithFields {};

struct Enum
    : NamedType
    , DependentType {
  struct Rule {
    using InputPattern = std::variant<Value, Star>;

    std::vector<InputPattern> inputs = {};
    std::vector<Constructor> outputs = {};
  };

  std::vector<Rule> pattern_mapping = {};
};

struct AST {
  std::unordered_map<InternedString, Message> messages;
  std::unordered_map<InternedString, Enum> enums;
};

} // namespace dbuf::ast
