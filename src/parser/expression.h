#pragma once

#include <memory>
#include <string>
#include <vector>

namespace dbuf::parser {

class Primary {
public:
  virtual ~Primary() = default;
};

class Expression {
public:
  virtual ~Expression() = default;
};

struct TypeExpression {
  std::string type_name;
  std::vector<std::shared_ptr<Expression>> type_parameters;
};

class BinaryExpression : public Expression {
public:
  BinaryExpression(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right);

protected:
  std::shared_ptr<Expression> left_;
  std::shared_ptr<Expression> right_;
};

class PlusExpression : public BinaryExpression {
public:
  PlusExpression(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right);
};

class MinusExpression : public BinaryExpression {
public:
  MinusExpression(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right);
};

class StarExpression : public BinaryExpression {
public:
  StarExpression(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right);
};

class SlashExpression : public BinaryExpression {
public:
  SlashExpression(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right);
};

class BangEqualExpression : public BinaryExpression {
public:
  BangEqualExpression(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right);
};

class GreaterEqualExpression : public BinaryExpression {
public:
  GreaterEqualExpression(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right);
};

class LessEqualExpression : public BinaryExpression {
public:
  LessEqualExpression(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right);
};

class AndExpression : public BinaryExpression {
public:
  AndExpression(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right);
};

class OrExpression : public BinaryExpression {
public:
  OrExpression(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right);
};

class LessExpression : public BinaryExpression {
public:
  LessExpression(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right);
};

class EqualExpression : public BinaryExpression {
public:
  EqualExpression(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right);
};

class GreaterExpression : public BinaryExpression {
public:
  GreaterExpression(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right);
};

class UnaryExpression : public Expression {
public:
  explicit UnaryExpression(std::shared_ptr<Expression> expression);

protected:
  std::shared_ptr<Expression> expression_;
};

class UnaryMinusExpression : public UnaryExpression {
public:
  explicit UnaryMinusExpression(std::shared_ptr<Expression> expression);
};

class UnaryBangExpression : public UnaryExpression {
public:
  explicit UnaryBangExpression(std::shared_ptr<Expression> expression);
};

// Value class definitions
class Value : public Primary {
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
  FieldInitialization() = default;
  void AddField(std::string field_identifier, std::unique_ptr<Expression> expression);

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
class FieldAccess : public Primary {
public:
  FieldAccess(std::string &var_identifier, std::string &field_identifier);

  FieldAccess(std::unique_ptr<FieldAccess> field_access, std::string &field_identifier);

private:
  std::unique_ptr<FieldAccess> field_access_;
  std::string var_identifier_;
  std::string field_identifier_;
};

} // namespace dbuf::parser
