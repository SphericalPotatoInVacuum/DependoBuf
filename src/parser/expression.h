#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace dbuf::parser {

class Expression {
public:
  virtual ~Expression() = default;
};

struct TypeExpression : Expression {
  explicit TypeExpression(std::string type)
      : type_name(std::move(type)) {}
  std::string type_name;
  std::vector<std::unique_ptr<Expression>> type_parameters;
};

enum class BinaryExpressionType {
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

class BinaryExpression : public Expression {
public:
  BinaryExpression(
      std::unique_ptr<Expression> left,
      BinaryExpressionType type,
      std::unique_ptr<Expression> right);

protected:
  std::unique_ptr<Expression> left_;
  BinaryExpressionType type_;
  std::unique_ptr<Expression> right_;
};

enum class UnaryExpressionType { kMinus, kBang };

class UnaryExpression : public Expression {
public:
  explicit UnaryExpression(UnaryExpressionType type, std::unique_ptr<Expression> expression);

protected:
  UnaryExpressionType type_;
  std::unique_ptr<Expression> expression_;
};

// Value class definitions
class Value : public Expression {
public:
  ~Value() override = default;
};

class BoolValue : public Value {
public:
  explicit BoolValue(bool value);

private:
  bool value_;
};

class FloatValue : public Value {
public:
  explicit FloatValue(float value);

private:
  float value_;
};

class IntValue : public Value {
public:
  explicit IntValue(int value);

private:
  int value_;
};

class StringValue : public Value {
public:
  explicit StringValue(std::string value);

private:
  std::string value_;
};

// Field initialization class definition
class FieldInitialization {
public:
  void AddField(const std::string &field_identifier, std::unique_ptr<Expression> expression);

private:
  std::vector<std::pair<std::string, std::unique_ptr<Expression>>> fields_;
};

// Constructed value class definition
class ConstructedValue : public Value {
public:
  ConstructedValue(
      std::string constructor_identifier,
      std::unique_ptr<FieldInitialization> field_initialization);

private:
  std::string constructor_identifier_;
  std::unique_ptr<FieldInitialization> field_initialization_;
};

// Field access class definition
struct VarAccess : public Expression {
  explicit VarAccess(std::string v)
      : var_identifier(std::move(v)) {}
  std::string var_identifier;
  std::vector<std::string> field_identifiers;
};

} // namespace dbuf::parser
