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
struct ScalarValue : ASTNode {
  T value = {};
};

struct Expression;

/**
 * @brief Represents variable access expressions,
 * like `foo.bar.baz`
 *
 */
struct VarAccess {
  Identifier var_identifier;
  std::vector<Identifier> field_identifiers = {};
};

/**
 * @brief Represents a type expression, like `Vec Int 5`
 *
 */
struct TypeExpression : ASTNode {
  Identifier identifier;
  std::vector<std::shared_ptr<const Expression>> parameters = {};
};

/**
 * @brief Represents a star value (`*`) that matches any value in pattern matching
 *
 */
struct Star : ASTNode {};

/**
 * @brief Represents a binary expression operator
 *
 */
enum struct BinaryExpressionType { Plus, Minus, Star, Slash, And, Or };

/**
 * @brief Represents a binary expression, like `1 + 2`
 *
 */
struct BinaryExpression : ASTNode {
  BinaryExpressionType type;
  std::shared_ptr<const Expression> left;
  std::shared_ptr<const Expression> right;
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
struct UnaryExpression : ASTNode {
  UnaryExpressionType type;
  std::shared_ptr<const Expression> expression;
};

/**
 * @brief Represents a constructed value, like `Succ{prev: Zero{}}`
 *
 */
struct ConstructedValue : ASTNode {
  Identifier constructor_identifier;
  std::vector<std::pair<Identifier, std::shared_ptr<const Expression>>> fields = {};
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
