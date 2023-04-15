#include "parser/ast.h"

#include <iostream>

namespace dbuf::parser {

TypedVariable::TypedVariable(std::string name, TypeExpression type_expression)
    : name_(std::move(name))
    , type_expression_(std::move(type_expression)) {}

Constructor::Constructor(std::string name)
    : name_(std::move(name)) {}

void Constructor::AddField(TypedVariable field) { fields_.emplace_back(std::move(field)); }

Message::Message(std::string name)
    : name_(std::move(name)) {}

void Message::AddDependency(TypedVariable type_dependency_) {
  type_dependencies_.insert(std::make_pair(type_dependency_.name_, std::move(type_dependency_)));
}

void Message::AddField(TypedVariable field) { fields_.emplace_back(std::move(field)); }

Enum::Enum(std::string name)
    : name_(std::move(name)) {}

void Enum::AddDependency(TypedVariable type_dependency_) {
  type_dependencies_.insert(std::make_pair(type_dependency_.name_, std::move(type_dependency_)));
}

void Enum::AddInput(std::vector<std::unique_ptr<std::variant<Value, StarValue>>> input) {
  inputs_.emplace_back(std::move(input));
}

void Enum::AddOutput(std::vector<Constructor> output) { outputs_.emplace_back(std::move(output)); }

void AST::AddMessage(Message message) {
  messages_.insert(std::make_pair(message.name_, std::move(message)));
}

void AST::AddEnum(Enum enum_) { enums_.insert(std::make_pair(enum_.name_, std::move(enum_))); }

} // namespace dbuf::parser
