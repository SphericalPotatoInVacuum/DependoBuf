#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace dbuf::parser {

template <typename T>
struct ScalarValue {
  T value_;
};

struct Expression;

// Field access struct definition
struct VarAccess {
  uint64_t var_identifier_;
  std::vector<uint64_t> field_identifiers_ = {};
};

// Type std::unique_ptr<Expression> struct definition
struct TypeExpression {
  TypeExpression() = default;

  TypeExpression(const TypeExpression &)            = delete;
  TypeExpression &operator=(const TypeExpression &) = delete;

  TypeExpression(TypeExpression &&)            = default;
  TypeExpression &operator=(TypeExpression &&) = default;

  uint64_t type_name_;
  std::vector<std::unique_ptr<Expression>> type_parameters_ = {};
};

// * value
struct StarValue {};

// Supported binary operations
enum struct BinaryExpressionType { kPlus, kMinus, kStar, kSlash, kAnd, kOr };

// Supported unary operations
enum struct UnaryExpressionType { kMinus, kBang };

struct BinaryExpression {
  std::unique_ptr<Expression> left_;
  BinaryExpressionType type_;
  std::unique_ptr<Expression> right_;
};

struct UnaryExpression {
  UnaryExpressionType type_;
  std::unique_ptr<Expression> expression_;
};

// Field initialization struct definition
struct FieldInitialization {
  FieldInitialization() = default;

  FieldInitialization(const FieldInitialization &)            = delete;
  FieldInitialization &operator=(const FieldInitialization &) = delete;

  FieldInitialization(FieldInitialization &&)            = default;
  FieldInitialization &operator=(FieldInitialization &&) = default;

  void AddField(uint64_t field_identifier, std::unique_ptr<Expression> expression);

  std::vector<std::pair<uint64_t, std::unique_ptr<Expression>>> fields_ = {};
};

// Constructed value struct definition
struct ConstructedValue {
  uint64_t constructor_identifier_;
  FieldInitialization field_initialization_;
};

// Value definition
using Value = std::variant<
    ScalarValue<bool>,
    ScalarValue<double>,
    ScalarValue<int64_t>,
    ScalarValue<uint64_t>,
    ScalarValue<std::string>,
    ConstructedValue>;

struct Expression
    : std::variant<BinaryExpression, UnaryExpression, TypeExpression, Value, VarAccess> {
  using Base = std::variant<BinaryExpression, UnaryExpression, TypeExpression, Value, VarAccess>;
  using Base::Base;
};

} // namespace dbuf::parser
