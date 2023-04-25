#pragma once

#include "core/interning/interned_string.h"

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <variant>
#include <vector>
namespace dbuf::ast {

/**
 * @brief Represents scalar values like numbers, booleans and strings
 *
 * @tparam T The type of the value
 */
template <typename T>
struct ScalarValue {
  T value = {};
};

struct Expression;

/**
 * @brief Represents variable access expressions,
 * like `foo.bar.baz`
 *
 */
struct VarAccess {
  InternedString var_identifier;
  std::vector<InternedString> field_identifiers = {};
};

/**
 * @brief Represents a type expression, like `Vec Int 5`
 *
 */
struct TypeExpression {
  InternedString name;
  std::vector<std::unique_ptr<Expression>> parameters = {};
};

/**
 * @brief Represents a star value (`*`) that matches any value in pattern matching
 *
 */
struct Star {};

/**
 * @brief Represents a binary expression operator
 *
 */
enum struct BinaryExpressionType { Plus, Minus, Star, Slash, And, Or };

/**
 * @brief Represents a binary expression, like `1 + 2`
 *
 */
struct BinaryExpression {
  BinaryExpressionType type;
  std::unique_ptr<Expression> left;
  std::unique_ptr<Expression> right;
};

/**
 * @brief Represents a unary expression operator
 *
 */
enum struct UnaryExpressionType { Minus, Bang };

/**
 * @brief Represents a unary expression, like `-(m + n)`
 *
 */
struct UnaryExpression {
  UnaryExpressionType type;
  std::unique_ptr<Expression> expression;
};

/**
 * @brief Represents a constructed value, like `Succ{prev: Zero{}}`
 *
 */
struct ConstructedValue {
  InternedString constructor_identifier;
  std::vector<std::pair<InternedString, std::unique_ptr<Expression>>> fields = {};
};

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

/**
 * @brief Represents an expression, which can be a binary or a unary expression, a type expression,
 * a value or a variable access
 *
 */
struct Expression
    : std::variant<BinaryExpression, UnaryExpression, TypeExpression, Value, VarAccess> {
  using Base = std::variant<BinaryExpression, UnaryExpression, TypeExpression, Value, VarAccess>;
  using Base::Base;
};

} // namespace dbuf::ast
