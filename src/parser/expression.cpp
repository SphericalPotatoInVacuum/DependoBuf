#include <memory>
#include <parser/expression.h>
#include <string>
#include <utility>
#include <vector>

namespace dbuf::parser {

BinaryExpression::BinaryExpression(
    std::shared_ptr<Expression> left, std::shared_ptr<Expression> right)
    : left_(std::move(left)), right_(std::move(right)) {}

PlusExpression::PlusExpression(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right)
    : BinaryExpression(std::move(left), std::move(right)) {}

MinusExpression::MinusExpression(
    std::shared_ptr<Expression> left, std::shared_ptr<Expression> right)
    : BinaryExpression(std::move(left), std::move(right)) {}

StarExpression::StarExpression(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right)
    : BinaryExpression(std::move(left), std::move(right)) {}

SlashExpression::SlashExpression(
    std::shared_ptr<Expression> left, std::shared_ptr<Expression> right)
    : BinaryExpression(std::move(left), std::move(right)) {}

BangEqualExpression::BangEqualExpression(
    std::shared_ptr<Expression> left, std::shared_ptr<Expression> right)
    : BinaryExpression(std::move(left), std::move(right)) {}

GreaterEqualExpression::GreaterEqualExpression(
    std::shared_ptr<Expression> left, std::shared_ptr<Expression> right)
    : BinaryExpression(std::move(left), std::move(right)) {}

LessEqualExpression::LessEqualExpression(
    std::shared_ptr<Expression> left, std::shared_ptr<Expression> right)
    : BinaryExpression(std::move(left), std::move(right)) {}

AndExpression::AndExpression(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right)
    : BinaryExpression(std::move(left), std::move(right)) {}

OrExpression::OrExpression(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right)
    : BinaryExpression(std::move(left), std::move(right)) {}

LessExpression::LessExpression(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right)
    : BinaryExpression(std::move(left), std::move(right)) {}

EqualExpression::EqualExpression(
    std::shared_ptr<Expression> left, std::shared_ptr<Expression> right)
    : BinaryExpression(std::move(left), std::move(right)) {}

GreaterExpression::GreaterExpression(
    std::shared_ptr<Expression> left, std::shared_ptr<Expression> right)
    : BinaryExpression(std::move(left), std::move(right)) {}

UnaryExpression::UnaryExpression(std::shared_ptr<Expression> expression)
    : expression_(std::move(std::move(expression))) {}

UnaryMinusExpression::UnaryMinusExpression(std::shared_ptr<Expression> expression)
    : UnaryExpression(std::move(expression)) {}

UnaryBangExpression::UnaryBangExpression(std::shared_ptr<Expression> expression)
    : UnaryExpression(std::move(expression)) {}

BoolValue::BoolValue(bool value) : value_(value) {}

FloatValue::FloatValue(float value) : value_(value) {}

IntValue::IntValue(int value) : value_(value) {}

StringValue::StringValue(std::string value) : value_(std::move(std::move(value))) {}

// Field initialization class definition
void FieldInitialization::AddField(
    const std::string &field_identifier, std::unique_ptr<Expression> expression) {
  fields_.emplace_back(field_identifier, std::move(expression));
}

ConstructedValue::ConstructedValue(
    std::string constructor_identifier, std::unique_ptr<FieldInitialization> field_initialization)
    : constructor_identifier_(std::move(std::move(constructor_identifier))),
      field_initialization_(std::move(field_initialization)) {}

FieldAccess::FieldAccess(std::string &var_identifier, std::string &field_identifier)
    : field_access_(nullptr), var_identifier_(var_identifier), field_identifier_(field_identifier) {
}

FieldAccess::FieldAccess(std::unique_ptr<FieldAccess> field_access, std::string &field_identifier)
    : field_access_(std::move(field_access)), field_identifier_(field_identifier) {}

} // namespace dbuf::parser
