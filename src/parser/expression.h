#pragma once

#include <memory>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace dbuf::parser {

struct ConstructedValue;
// * value
struct StarValue {};

template <typename T>
struct ScalarValue {
  T value_;
};

// Value definition
using Value = std::variant<
  ScalarValue<bool>, ScalarValue<double>,
  ScalarValue<long long>, ScalarValue<unsigned>,
  ScalarValue <std::string>, ConstructedValue
>;

// Field access struct definition
struct VarAccess {
  std::string var_identifier;
  std::vector<std::string> field_identifiers;
};

struct TypeExpression;
struct BinaryExpression;
struct UnaryExpression;

using Expression = std::variant<BinaryExpression, UnaryExpression, TypeExpression, Value, VarAccess>;

// Type std::unique_ptr<Expression> struct definition
struct TypeExpression {
  std::string type_name_;
  std::vector<Expression> type_parameters_ = {};
};

// Supported binary operations
enum struct BinaryExpressionType { kPlus, kMinus, kStar, kSlash, kAnd, kOr };

// Supported unary operations
enum struct UnaryExpressionType { kMinus, kBang };

struct BinaryExpression {
  Expression left_;
  BinaryExpressionType type_;
  Expression right_;
};

struct UnaryExpression {
  UnaryExpressionType type_;
  Expression expression_;
};

// Field initialization struct definition
struct FieldInitialization {
  void AddField(const std::string &field_identifier, Expression& expression);

  std::vector<std::pair<std::string, Expression>> fields_;
};


// Constructed value struct definition
struct ConstructedValue {
  std::string constructor_identifier_;
  FieldInitialization field_initialization_;
};

} // namespace dbuf::parser
