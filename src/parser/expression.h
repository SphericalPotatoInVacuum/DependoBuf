#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace dbuf::parser {

struct Expression {
  virtual ~Expression() = default;
};

struct TypeExpression : Expression {
  TypeExpression() = default;
  explicit TypeExpression(std::string type)
      : type_name(std::move(type)) {}

  std::string type_name;
  std::vector<std::unique_ptr<Expression>> type_parameters = {};
};

enum struct BinaryExpressionType {
  kPlus,
  kMinus,
  kStar,
  kSlash,
  kBangEqual,
  kLess,
  kLessEqual,
  kEqual,
  kGreaterEqual,
  kGreater,
  kAnd,
  kOr
};

struct BinaryExpression : public Expression {
  BinaryExpression(
      std::unique_ptr<Expression> left,
      BinaryExpressionType type,
      std::unique_ptr<Expression> right);

  std::unique_ptr<Expression> left_;
  BinaryExpressionType type_;
  std::unique_ptr<Expression> right_;
};

enum struct UnaryExpressionType { kMinus, kBang };

struct UnaryExpression : public Expression {
  UnaryExpression(UnaryExpressionType type, std::unique_ptr<Expression> expression);

  UnaryExpressionType type_;
  std::unique_ptr<Expression> expression_;
};

// Value struct definitions
struct Value : public Expression {};

struct BoolValue : public Value {
  explicit BoolValue(bool value);

  bool value_;
};

struct FloatValue : public Value {
  explicit FloatValue(float value);

  float value_;
};

struct IntValue : public Value {
  explicit IntValue(int value);

  int value_;
};

struct StringValue : public Value {
  explicit StringValue(std::string value);

  std::string value_;
};

// Field initialization struct definition
struct FieldInitialization {
  void AddField(const std::string &field_identifier, std::unique_ptr<Expression> expression);

  std::vector<std::pair<std::string, std::unique_ptr<Expression>>> fields_;
};

// Constructed value struct definition
struct ConstructedValue : public Value {
  ConstructedValue(std::string constructor_identifier, FieldInitialization field_initialization);

  std::string constructor_identifier_;
  FieldInitialization field_initialization_;
};

struct StarValue {};

// Field access struct definition
struct VarAccess : public Expression {
  explicit VarAccess(std::string v)
      : var_identifier(std::move(v)) {}
  std::string var_identifier;
  std::vector<std::string> field_identifiers;
};

} // namespace dbuf::parser
