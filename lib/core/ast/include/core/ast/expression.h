#pragma once

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
  uint64_t var_identifier;
  std::vector<uint64_t> field_identifiers = {};
};

/**
 * @brief Represents a type expression, like `Vec Int 5`
 *
 */
struct TypeExpression {
  uint64_t name;
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
enum struct BinaryExpressionType { kPlus, kMinus, kStar, kSlash, kAnd, kOr };

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
enum struct UnaryExpressionType { kMinus, kBang };

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
  uint64_t constructor_identifier;
  std::vector<std::pair<uint64_t, std::unique_ptr<Expression>>> fields = {};
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
