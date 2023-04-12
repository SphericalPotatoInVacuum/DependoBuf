#include "expression.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace dbuf::parser {

BinaryExpression::BinaryExpression(
    std::unique_ptr<Expression> left,
    BinaryExpressionType type,
    std::unique_ptr<Expression> right)
    : left_(std::move(left))
    , type_(std::move(type))
    , right_(std::move(right)) {}

UnaryExpression::UnaryExpression(UnaryExpressionType type, std::unique_ptr<Expression> expression)
    : type_(std::move(type))
    , expression_(std::move(std::move(expression))) {}

BoolValue::BoolValue(bool value)
    : value_(value) {}

FloatValue::FloatValue(float value)
    : value_(value) {}

IntValue::IntValue(int value)
    : value_(value) {}

StringValue::StringValue(std::string value)
    : value_(std::move(std::move(value))) {}

// Field initialization class definition
void FieldInitialization::AddField(
    const std::string &field_identifier,
    std::unique_ptr<Expression> expression) {
  fields_.emplace_back(field_identifier, std::move(expression));
}

ConstructedValue::ConstructedValue(
    std::string constructor_identifier,
    std::unique_ptr<FieldInitialization> field_initialization)
    : constructor_identifier_(std::move(std::move(constructor_identifier)))
    , field_initialization_(std::move(field_initialization)) {}

} // namespace dbuf::parser
