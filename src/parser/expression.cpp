#include "expression.h"

#include <memory>
#include <string>
#include <vector>

namespace dbuf {
namespace parser {

Expression::~Expression() {}

BinaryExpression::BinaryExpression(
    std::shared_ptr<Expression> left, std::shared_ptr<Expression> right)
    : left_(left), right_(right) {}

PlusExpression::PlusExpression(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right)
    : BinaryExpression(left, right) {}

std::shared_ptr<Primary> PlusExpression::Evaluate() {
  std::shared_ptr<Primary> left  = left_->Evaluate();
  std::shared_ptr<Primary> right = right_->Evaluate();

  return left->operator+(right);
}

MinusExpression::MinusExpression(
    std::shared_ptr<Expression> left, std::shared_ptr<Expression> right)
    : BinaryExpression(left, right) {}

std::shared_ptr<Primary> MinusExpression::Evaluate() {
  std::shared_ptr<Primary> left  = left_->Evaluate();
  std::shared_ptr<Primary> right = right_->Evaluate();

  return left->operator-(right);
}

StarExpression::StarExpression(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right)
    : BinaryExpression(left, right) {}
std::shared_ptr<Primary> StarExpression::Evaluate() {
  std::shared_ptr<Primary> left  = left_->Evaluate();
  std::shared_ptr<Primary> right = right_->Evaluate();

  return left->operator*(right);
}

SlashExpression::SlashExpression(
    std::shared_ptr<Expression> left, std::shared_ptr<Expression> right)
    : BinaryExpression(left, right) {}
std::shared_ptr<Primary> SlashExpression::Evaluate() {
  std::shared_ptr<Primary> left  = left_->Evaluate();
  std::shared_ptr<Primary> right = right_->Evaluate();

  return left->operator/(right);
}

BangEqualExpression::BangEqualExpression(
    std::shared_ptr<Expression> left, std::shared_ptr<Expression> right)
    : BinaryExpression(left, right) {}
std::shared_ptr<Primary> BangEqualExpression::Evaluate() {
  std::shared_ptr<Primary> left  = left_->Evaluate();
  std::shared_ptr<Primary> right = right_->Evaluate();

  return left->operator!=(right);
}

GreaterEqualExpression::GreaterEqualExpression(
    std::shared_ptr<Expression> left, std::shared_ptr<Expression> right)
    : BinaryExpression(left, right) {}
std::shared_ptr<Primary> GreaterEqualExpression::Evaluate() {
  std::shared_ptr<Primary> left  = left_->Evaluate();
  std::shared_ptr<Primary> right = right_->Evaluate();

  return left->operator>=(right);
}

LessEqualExpression::LessEqualExpression(
    std::shared_ptr<Expression> left, std::shared_ptr<Expression> right)
    : BinaryExpression(left, right) {}
std::shared_ptr<Primary> LessEqualExpression::Evaluate() {
  std::shared_ptr<Primary> left  = left_->Evaluate();
  std::shared_ptr<Primary> right = right_->Evaluate();

  return left->operator<=(right);
}

AndExpression::AndExpression(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right)
    : BinaryExpression(left, right) {}
std::shared_ptr<Primary> AndExpression::Evaluate() {
  std::shared_ptr<Primary> left  = left_->Evaluate();
  std::shared_ptr<Primary> right = right_->Evaluate();

  return left->operator&&(right);
}

OrExpression::OrExpression(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right)
    : BinaryExpression(left, right) {}
std::shared_ptr<Primary> OrExpression::Evaluate() {
  std::shared_ptr<Primary> left  = left_->Evaluate();
  std::shared_ptr<Primary> right = right_->Evaluate();

  return left->operator||(right);
}

LessExpression::LessExpression(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right)
    : BinaryExpression(left, right) {}
std::shared_ptr<Primary> LessExpression::Evaluate() {
  std::shared_ptr<Primary> left  = left_->Evaluate();
  std::shared_ptr<Primary> right = right_->Evaluate();

  return left->operator<(right);
}

EqualExpression::EqualExpression(
    std::shared_ptr<Expression> left, std::shared_ptr<Expression> right)
    : BinaryExpression(left, right) {}
std::shared_ptr<Primary> EqualExpression::Evaluate() {
  std::shared_ptr<Primary> left  = left_->Evaluate();
  std::shared_ptr<Primary> right = right_->Evaluate();

  return left->operator==(right);
}

GreaterExpression::GreaterExpression(
    std::shared_ptr<Expression> left, std::shared_ptr<Expression> right)
    : BinaryExpression(left, right) {}
std::shared_ptr<Primary> GreaterExpression::Evaluate() {
  std::shared_ptr<Primary> left  = left_->Evaluate();
  std::shared_ptr<Primary> right = right_->Evaluate();

  return left->operator>(right);
}

UnaryExpression::UnaryExpression(std::shared_ptr<Expression> expression)
    : expression_(expression) {}

UnaryMinusExpression::UnaryMinusExpression(std::shared_ptr<Expression> expression)
    : UnaryExpression(expression) {}
std::shared_ptr<Primary> UnaryMinusExpression::Evaluate() {
  return (expression_->Evaluate())->operator+();
}

UnaryBangExpression::UnaryBangExpression(std::unique_ptr<Expression> expression)
    : UnaryExpression(std::move(expression)) {}
std::shared_ptr<Primary> UnaryBangExpression::Evaluate() {
  return (expression_->Evaluate())->operator!();
}

Primary::~Primary() {}

Value::~Value() {}

BoolLiteral::BoolLiteral(bool value) : value_(value) {}

BoolLiteral::operator bool() const { return value_; }

// Boolean operators
std::shared_ptr<Primary> BoolLiteral::operator!() const {
  return std::make_shared<Primary>(!value_);
} // Logical NOT operator
std::shared_ptr<Primary> BoolLiteral::operator&&(const std::shared_ptr<Primary> other) const {
  bool other_value = *std::static_pointer_cast<bool>(other);
  return std::make_shared<Primary>(value_ && other_value);
} // Logical AND operator
std::shared_ptr<Primary> BoolLiteral::operator||(const std::shared_ptr<Primary> other) const {
  bool other_value = *std::static_pointer_cast<bool>(other);
  return std::make_shared<Primary>(value_ || other_value);
} // Logical OR operator

// Comparison operators
std::shared_ptr<Primary> BoolLiteral::operator==(const std::shared_ptr<Primary> other) const {
  bool other_value = *std::static_pointer_cast<bool>(other);
  return std::make_shared<Primary>(value_ == other_value);
}
std::shared_ptr<Primary> BoolLiteral::operator!=(const std::shared_ptr<Primary> other) const {
  bool other_value = *std::static_pointer_cast<bool>(other);
  return std::make_shared<Primary>(value_ != other_value);
}
std::shared_ptr<Primary> BoolLiteral::operator<(const std::shared_ptr<Primary> other) const {
  bool other_value = *std::static_pointer_cast<bool>(other);
  return std::make_shared<Primary>(value_ < other_value);
}
std::shared_ptr<Primary> BoolLiteral::operator<=(const std::shared_ptr<Primary> other) const {
  bool other_value = *std::static_pointer_cast<bool>(other);
  return std::make_shared<Primary>(value_ <= other_value);
}
std::shared_ptr<Primary> BoolLiteral::operator>(const std::shared_ptr<Primary> other) const {
  bool other_value = *std::static_pointer_cast<bool>(other);
  return std::make_shared<Primary>(value_ > other_value);
}
std::shared_ptr<Primary> BoolLiteral::operator>=(const std::shared_ptr<Primary> other) const {
  bool other_value = *std::static_pointer_cast<bool>(other);
  return std::make_shared<Primary>(value_ >= other_value);
}

FloatLiteral::FloatLiteral(float value) : value_(value) {}

FloatLiteral::operator float() const { return value_; }

// Unary arithmetic operators
std::shared_ptr<Primary> FloatLiteral::operator+() const {
  return std::make_shared<Primary>(+value_);
} // Unary plus operator
std::shared_ptr<Primary> FloatLiteral::operator-() const {
  return std::make_shared<Primary>(-value_);
} // Unary minus operator

// Arithmetic operators
std::shared_ptr<Primary> FloatLiteral::operator+(const std::shared_ptr<Primary> other) const {
  return std::make_shared<Primary>(value_ + *std::static_pointer_cast<float>(other));
} // Addition operator
std::shared_ptr<Primary> FloatLiteral::operator-(const std::shared_ptr<Primary> other) const {
  return std::make_shared<Primary>(value_ - *std::static_pointer_cast<float>(other));
} // Subtraction operator
std::shared_ptr<Primary> FloatLiteral::operator*(const std::shared_ptr<Primary> other) const {
  return std::make_shared<Primary>(value_ * *std::static_pointer_cast<float>(other));
} // Multiplication operator
std::shared_ptr<Primary> FloatLiteral::operator/(const std::shared_ptr<Primary> other) const {
  return std::make_shared<Primary>(value_ / *std::static_pointer_cast<float>(other));
} // Division operator

IntLiteral::IntLiteral(int value) : value_(value) {}

IntLiteral::operator int() const { return value_; }

// Unary arithmetic operators
std::shared_ptr<Primary> IntLiteral::operator+() const {
  return std::make_shared<Primary>(+value_);
} // Unary plus operator
std::shared_ptr<Primary> IntLiteral::operator-() const {
  return std::make_shared<Primary>(-value_);
} // Unary minus operator

// Arithmetic operators
std::shared_ptr<Primary> IntLiteral::operator+(const std::shared_ptr<Primary> other) const {
  return std::make_shared<Primary>(value_ + *std::static_pointer_cast<int>(other));
} // Addition operator
std::shared_ptr<Primary> IntLiteral::operator-(const std::shared_ptr<Primary> other) const {
  return std::make_shared<Primary>(value_ - *std::static_pointer_cast<int>(other));
} // Subtraction operator
std::shared_ptr<Primary> IntLiteral::operator*(const std::shared_ptr<Primary> other) const {
  return std::make_shared<Primary>(value_ * *std::static_pointer_cast<int>(other));
} // Multiplication operator
std::shared_ptr<Primary> IntLiteral::operator/(const std::shared_ptr<Primary> other) const {
  return std::make_shared<Primary>(value_ / *std::static_pointer_cast<int>(other));
} // Division operator

StringLiteral::StringLiteral(std::string value) : value_(value) {}

StringLiteral::operator std::string() const { return value_; }

// Field initialization class definition
FieldInitialization::FieldInitialization() {}
void FieldInitialization::AddField(
    std::string field_identifier, std::unique_ptr<Expression> expression) {
  fields_.push_back(make_pair(field_identifier, std::move(expression)));
}

ConstructedValue::ConstructedValue(
    std::string constructor_identifier, std::unique_ptr<FieldInitialization> field_initialization)
    : constructor_identifier_(constructor_identifier),
      field_initialization_(std::move(field_initialization)) {}

FieldAccess::FieldAccess(std::string &var_identifier, std::string &field_identifier)
    : field_access_(nullptr), var_identifier_(var_identifier), field_identifier_(field_identifier) {
}

FieldAccess::FieldAccess(std::unique_ptr<FieldAccess> field_access, std::string &field_identifier)
    : field_access_(std::move(field_access)), var_identifier_(""),
      field_identifier_(field_identifier) {}

} // namespace parser
} // namespace dbuf
