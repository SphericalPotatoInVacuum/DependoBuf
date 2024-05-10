/*
This file is part of DependoBuf project.

Copyright (C) 2023 Alexander Bogdanov, Alice Vernigor

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/
#pragma once

#include "core/ast/expression.h"
#include "core/interning/interned_string.h"

#include <cstdint>
#include <memory>
#include <ostream>
#include <string>
#include <unordered_map>
#include <utility>
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

    std::vector<InputPattern> inputs;
    std::vector<Constructor> outputs;
  };

  std::vector<Rule> pattern_mapping;
};

struct Func
    : Identifiable
    , DependentType {
  TypeExpression return_type;
  ExprPtr return_value;
};

struct AST {
  std::unordered_map<InternedString, std::variant<Message, Enum>> types;
  std::unordered_map<InternedString, InternedString> constructor_to_type;
  std::unordered_map<InternedString, Func> functions;
};

inline std::ostream &operator<<(std::ostream &os, const TypedVariable &var) {
  os << var.name << " " << var.type_expression;
  return os;
}

inline std::ostream &operator<<(std::ostream &os, const Enum::Rule::InputPattern &pattern) {
  std::visit([&os](const auto &input) { os << input; }, pattern);
  return os;
}

inline std::ostream &operator<<(std::ostream &os, const std::vector<Enum::Rule::InputPattern> &inputs) {
  bool first = true;
  for (const auto &input : inputs) {
    if (!first) {
      os << ", ";
    }
    first = false;
    os << input;
  }
  return os;
}

} // namespace dbuf::ast
