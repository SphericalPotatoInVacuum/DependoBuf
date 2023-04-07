#pragma once

#include <memory>
#include <string>
#include <vector>

namespace dbuf {
namespace parser {

struct TypeExpression {
  std::string type_name;
  std::vector<std::shared_ptr<Expression>> type_parameters;
};

class Expression {
public:
  virtual ~Expression() {}
  virtual std::shared_ptr<Primary> Evaluate() = 0;
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

  std::shared_ptr<Primary> Evaluate() override;
};

class MinusExpression : public BinaryExpression {
public:
  MinusExpression(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right);

  std::shared_ptr<Primary> Evaluate() override;
};

class StarExpression : public BinaryExpression {
public:
  StarExpression(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right);
  std::shared_ptr<Primary> Evaluate() override;
};

class SlashExpression : public BinaryExpression {
public:
  SlashExpression(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right);
  std::shared_ptr<Primary> Evaluate() override;
};

class BangEqualExpression : public BinaryExpression {
public:
  BangEqualExpression(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right);
  std::shared_ptr<Primary> Evaluate() override;
};

class GreaterEqualExpression : public BinaryExpression {
public:
  GreaterEqualExpression(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right);
  std::shared_ptr<Primary> Evaluate();
};

class LessEqualExpression : public BinaryExpression {
public:
  LessEqualExpression(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right);
  std::shared_ptr<Primary> Evaluate() override;
};

class AndExpression : public BinaryExpression {
public:
  AndExpression(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right);
  std::shared_ptr<Primary> Evaluate() override;
};

class OrExpression : public BinaryExpression {
public:
  OrExpression(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right);
  std::shared_ptr<Primary> Evaluate() override;
};

class LessExpression : public BinaryExpression {
public:
  LessExpression(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right);
  std::shared_ptr<Primary> Evaluate() override;
};

class EqualExpression : public BinaryExpression {
public:
  EqualExpression(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right);
  std::shared_ptr<Primary> Evaluate() override;
};

class GreaterExpression : public BinaryExpression {
public:
  GreaterExpression(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right);
  std::shared_ptr<Primary> Evaluate() override;
};

class UnaryExpression : public Expression {
public:
  UnaryExpression(std::shared_ptr<Expression> expression);

protected:
  std::shared_ptr<Expression> expression_;
};

class UnaryMinusExpression : public UnaryExpression {
public:
  UnaryMinusExpression(std::shared_ptr<Expression> expression);
  std::shared_ptr<Primary> Evaluate();
};

class UnaryBangExpression : public UnaryExpression {
public:
  UnaryBangExpression(std::unique_ptr<Expression> expression);
  std::shared_ptr<Primary> Evaluate() override;
};

class Primary {
public:
  virtual ~Primary();

  // Boolean operators
  virtual std::shared_ptr<Primary> operator!() const = 0; // Logical NOT operator
  virtual std::shared_ptr<Primary>
  operator&&(const std::shared_ptr<Primary> other) const = 0; // Logical AND operator
  virtual std::shared_ptr<Primary>
  operator||(const std::shared_ptr<Primary> other) const = 0; // Logical OR operator

  // Comparison operators
  virtual std::shared_ptr<Primary> operator==(const std::shared_ptr<Primary> other) const = 0;
  virtual std::shared_ptr<Primary> operator!=(const std::shared_ptr<Primary> other) const = 0;
  virtual std::shared_ptr<Primary> operator<(const std::shared_ptr<Primary> other) const  = 0;
  virtual std::shared_ptr<Primary> operator<=(const std::shared_ptr<Primary> other) const = 0;
  virtual std::shared_ptr<Primary> operator>(const std::shared_ptr<Primary> other) const  = 0;
  virtual std::shared_ptr<Primary> operator>=(const std::shared_ptr<Primary> other) const = 0;

  // Arithmetic operators
  virtual std::shared_ptr<Primary>
  operator+(const std::shared_ptr<Primary> other) const = 0; // Addition operator
  virtual std::shared_ptr<Primary>
  operator-(const std::shared_ptr<Primary> other) const = 0; // Subtraction operator
  virtual std::shared_ptr<Primary>
  operator*(const std::shared_ptr<Primary> other) const = 0; // Multiplication operator
  virtual std::shared_ptr<Primary>
  operator/(const std::shared_ptr<Primary> other) const = 0; // Division operator

  // Unary arithmetic operators
  virtual std::shared_ptr<Primary> operator+() const = 0; // Unary plus operator
  virtual std::shared_ptr<Primary> operator-() const = 0; // Unary minus operator

  virtual std::shared_ptr<Primary> Evaluate() const = 0;
};

// Value class definition
class Value : public Primary {
public:
  virtual operator bool() const        = 0;
  virtual operator float() const       = 0;
  virtual operator std::string() const = 0;
  virtual operator int() const         = 0;
  virtual ~Value() {}
};

// Literal value class definitions
class BoolLiteral : public Value {
public:
  BoolLiteral(bool value);

  operator bool() const override;

  std::shared_ptr<Primary> operator!() const override;
  std::shared_ptr<Primary> operator&&(const std::shared_ptr<Primary> other) const override;
  std::shared_ptr<Primary> operator||(const std::shared_ptr<Primary> other) const override;
  std::shared_ptr<Primary> operator==(const std::shared_ptr<Primary> other) const override;
  std::shared_ptr<Primary> operator!=(const std::shared_ptr<Primary> other) const override;
  std::shared_ptr<Primary> operator<(const std::shared_ptr<Primary> other) const override;
  std::shared_ptr<Primary> operator<=(const std::shared_ptr<Primary> other) const override;
  std::shared_ptr<Primary> operator>(const std::shared_ptr<Primary> other) const override;
  std::shared_ptr<Primary> operator>=(const std::shared_ptr<Primary> other) const override;

private:
  bool value_;
};

class FloatLiteral : public Value {
public:
  FloatLiteral(float value);

  operator float() const override;

  std::shared_ptr<Primary> operator+() const override;
  std::shared_ptr<Primary> operator-() const override;
  std::shared_ptr<Primary> operator+(const std::shared_ptr<Primary> other) const override;
  std::shared_ptr<Primary> operator-(const std::shared_ptr<Primary> other) const override;
  std::shared_ptr<Primary> operator*(const std::shared_ptr<Primary> other) const override;
  std::shared_ptr<Primary> operator/(const std::shared_ptr<Primary> other) const override;

private:
  float value_;
};

class IntLiteral : public Value {
public:
  IntLiteral(int value);

  operator int() const override;

  std::shared_ptr<Primary> operator+() const override;
  std::shared_ptr<Primary> operator-() const override;
  std::shared_ptr<Primary> operator+(const std::shared_ptr<Primary> other) const override;
  std::shared_ptr<Primary> operator-(const std::shared_ptr<Primary> other) const override;
  std::shared_ptr<Primary> operator*(const std::shared_ptr<Primary> other) const override;
  std::shared_ptr<Primary> operator/(const std::shared_ptr<Primary> other) const override;

private:
  int value_;
};

class StringLiteral : public Value {
public:
  StringLiteral(std::string value);

  operator std::string() const override;

private:
  std::string value_;
};

// Field initialization class definition
class FieldInitialization {
public:
  FieldInitialization() {}
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

} // namespace parser
} // namespace dbuf
