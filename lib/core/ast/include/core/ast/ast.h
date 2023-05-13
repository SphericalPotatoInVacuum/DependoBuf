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
inline std::ostream &operator<<(std::ostream &os, const TypedVariable &var) {
  os << var.name << " " << var.type_expression;
  return os;
}

struct DependentType {
  std::vector<TypedVariable> type_dependencies = {};
};

struct TypeWithFields {
  std::vector<TypedVariable> fields = {};
};

struct Constructor
    : Identifiable
    , TypeWithFields {};

struct Message
    : Identifiable
    , DependentType
    , TypeWithFields {};

struct Enum
    : Identifiable
    , DependentType {
  struct Rule {
    using InputPattern = std::variant<Value, Star>;

    std::vector<InputPattern> inputs = {};
    std::vector<Constructor> outputs = {};
  };

  std::vector<Rule> pattern_mapping = {};
};

struct AST {
  std::unordered_map<InternedString, std::variant<Message, Enum>> types = {};
};

} // namespace dbuf::ast
