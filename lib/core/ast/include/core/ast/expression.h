/*
This file is part of DependoBuf project.

Copyright (C) 2023 Alexander Bogdanov, Alice Vernigor

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/
#pragma once

#include "core/interning/interned_string.h"
#include "location.hh"

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <variant>
#include <vector>
namespace dbuf::ast {

struct NamedType {
  InternedString name;
};

struct ASTNode {
  parser::location location;
};

struct Identifier
    : ASTNode
    , NamedType {};

struct Identifiable {
  Identifier identifier;
};

/**
 * @brief Represents scalar values like numbers, booleans and strings
 *
 * @tparam T The type of the value
 */
template <typename T>
struct ScalarValue : ASTNode { // NOLINT(bugprone-exception-escape)
  T value = {};
};
template <typename T>
std::ostream &operator<<(std::ostream &os, const ScalarValue<T> &val) {
  os << val.value;
  return os;
}
template <>
inline std::ostream &operator<<(std::ostream &os, const ScalarValue<std::string> &val) {
  os << "\"" << val.value << "\"";
  return os;
}

struct Expression;
std::ostream &operator<<(std::ostream &os, const Expression &expr);

/**
 * @brief Represents variable access expressions,
 * like `foo.bar.baz`
 *
 */
struct VarAccess {
  Identifier var_identifier;
  std::vector<Identifier> field_identifiers = {};
};
inline std::ostream &operator<<(std::ostream &os, const VarAccess &var) {
  os << var.var_identifier.name;
  for (const auto &field_identifier : var.field_identifiers) {
    os << "." << field_identifier.name;
  }
  return os;
}

/**
 * @brief Represents a type expression, like `Vec Int 5`
 *
 */
struct TypeExpression : ASTNode {
  Identifier identifier;
  std::vector<std::shared_ptr<const Expression>> parameters = {};
};
inline std::ostream &operator<<(std::ostream &os, const TypeExpression &expr) {
  os << expr.identifier.name;
  for (const auto &param : expr.parameters) {
    os << " " << *param;
  }
  return os;
}

/**
 * @brief Represents a star value (`*`) that matches any value in pattern matching
 *
 */
struct Star : ASTNode {};
inline std::ostream &operator<<(std::ostream &os, const Star & /*star*/) {
  os << "*";
  return os;
}

/**
 * @brief Represents a binary expression operator
 *
 */
enum struct BinaryExpressionType { Plus = '+', Minus = '-', Star = '*', Slash = '/', And = '&', Or = '|' };

/**
 * @brief Represents a binary expression, like `1 + 2`
 *
 */
struct BinaryExpression : ASTNode {
  BinaryExpressionType type;
  std::shared_ptr<const Expression> left;
  std::shared_ptr<const Expression> right;
};
inline std::ostream &operator<<(std::ostream &os, const BinaryExpression &expr) {
  os << "(" << *expr.left << " " << static_cast<char>(expr.type) << " " << *expr.right << ")";
  return os;
}

/**
 * @brief Represents a unary expression operator
 *
 */
enum struct UnaryExpressionType { Minus = '-', Bang = '!' };

/**
 * @brief Represents a unary expression, like `-(m + n)`
 *
 */
struct UnaryExpression : ASTNode {
  UnaryExpressionType type;
  std::shared_ptr<const Expression> expression;
};
inline std::ostream &operator<<(std::ostream &os, const UnaryExpression &expr) {
  os << static_cast<char>(expr.type) << *expr.expression;
  return os;
}

/**
 * @brief Represents a constructed value, like `Succ{prev: Zero{}}`
 *
 */
struct ConstructedValue : ASTNode {
  Identifier constructor_identifier;
  std::vector<std::pair<Identifier, std::shared_ptr<const Expression>>> fields = {};
};
inline std::ostream &operator<<(std::ostream &os, const ConstructedValue &val) {
  os << val.constructor_identifier.name << "{";
  bool first = true;
  for (const auto &[field_identifier, field_value] : val.fields) {
    if (first) {
      first = false;
    } else {
      os << ", ";
    }
    os << field_identifier.name << ": " << *field_value;
  }
  os << "}";
  return os;
}

/**
 * @brief Represents a value, which can be a scalar value or a constructed value
 *
 */
using Value = std::variant<
    ScalarValue<bool>,
    ScalarValue<double>,
    ScalarValue<int64_t>,
    ScalarValue<uint64_t>,
    ScalarValue<std::string>,
    ConstructedValue>;
inline std::ostream &operator<<(std::ostream &os, const Value &val) {
  std::visit([&os](const auto &val) { os << val; }, val);
  return os;
}

/**
 * @brief Represents an expression, which can be a binary or a unary expression, a type expression,
 * a value or a variable access
 *
 */
struct Expression : std::variant<BinaryExpression, UnaryExpression, TypeExpression, Value, VarAccess> {
  using Base = std::variant<BinaryExpression, UnaryExpression, TypeExpression, Value, VarAccess>;
  using Base::Base;
};

inline std::ostream &operator<<(std::ostream &os, const Expression &expr) {
  std::visit([&os](const auto &expr) { os << expr; }, expr);
  return os;
}

} // namespace dbuf::ast
