#include "parser/ast.h"

#include <iostream>

namespace dbuf::parser {

TypedVariable::TypedVariable(std::string name_, std::unique_ptr<TypeExpression> type_expression)
    : name(std::move(name_))
    , type_expression(std::move(type_expression)) {}

Constructor::Constructor(std::string name)
    : name_(std::move(name)) {}

void Constructor::AddField(std::unique_ptr<TypedVariable> field) {
  fields_.emplace_back(std::move(field));
}

Message::Message(std::string name)
    : name_(std::move(name)) {}

void Message::AddDependency(std::unique_ptr<TypedVariable> type_dependency_) {}

void Message::AddField(std::unique_ptr<TypedVariable> field) {
  fields_.emplace_back(std::move(field));
}

Enum::Enum(std::string &name)
    : name_(name) {}

void Enum::AddDependency(std::unique_ptr<TypedVariable> type_dependency_) {}

void Enum::AddInput(std::vector<std::unique_ptr<std::variant<Value, StarValue>>> input) {
  inputs_.emplace_back(std::move(input));
}

void Enum::AddOutput(std::vector<std::unique_ptr<Constructor>> output) {
  outputs_.emplace_back(std::move(output));
}

void AST::AddMessage(std::unique_ptr<Message> message) {
  messages_.insert(std::make_pair(message->name_, std::move(message)));
}

void AST::AddEnum(std::unique_ptr<Enum> enum_) {
  enums_.insert(std::make_pair(enum_->name_, std::move(enum_)));
}

} // namespace dbuf::parser
