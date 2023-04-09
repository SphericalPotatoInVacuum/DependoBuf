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
    : BinaryExpression(left, right) {}

StarExpression::StarExpression(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right)
    : BinaryExpression(left, right) {}

SlashExpression::SlashExpression(
    std::shared_ptr<Expression> left, std::shared_ptr<Expression> right)
    : BinaryExpression(left, right) {}

BangEqualExpression::BangEqualExpression(
    std::shared_ptr<Expression> left, std::shared_ptr<Expression> right)
    : BinaryExpression(left, right) {}

GreaterEqualExpression::GreaterEqualExpression(
    std::shared_ptr<Expression> left, std::shared_ptr<Expression> right)
    : BinaryExpression(left, right) {}

LessEqualExpression::LessEqualExpression(
    std::shared_ptr<Expression> left, std::shared_ptr<Expression> right)
    : BinaryExpression(left, right) {}

AndExpression::AndExpression(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right)
    : BinaryExpression(left, right) {}

OrExpression::OrExpression(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right)
    : BinaryExpression(left, right) {}

LessExpression::LessExpression(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right)
    : BinaryExpression(left, right) {}

EqualExpression::EqualExpression(
    std::shared_ptr<Expression> left, std::shared_ptr<Expression> right)
    : BinaryExpression(left, right) {}

GreaterExpression::GreaterExpression(
    std::shared_ptr<Expression> left, std::shared_ptr<Expression> right)
    : BinaryExpression(left, right) {}

UnaryExpression::UnaryExpression(std::shared_ptr<Expression> expression)
    : expression_(expression) {}

UnaryMinusExpression::UnaryMinusExpression(std::shared_ptr<Expression> expression)
    : UnaryExpression(expression) {}

UnaryBangExpression::UnaryBangExpression(std::shared_ptr<Expression> expression)
    : UnaryExpression(expression) {}

BoolValue::BoolValue(bool value) : value_(value) {}

FloatValue::FloatValue(float value) : value_(value) {}

IntValue::IntValue(int value) : value_(value) {}

StringValue::StringValue(std::string value) : value_(value) {}

// Field initialization class definition
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

} // namespace dbuf::parser
