#include "core/parser/ast.h"

#include <iostream>
#include <iterator>

namespace dbuf::parser {

void Constructor::AddField(TypedVariable field) {
  fields_.emplace_back(std::move(field));
}

void DependentType::AddDependency(TypedVariable type_dependency_) {
  type_dependencies_.emplace_back(std::move(type_dependency_));
}

void Message::AddField(TypedVariable field) {
  fields_.emplace_back(std::move(field));
}

void Enum::AddInput(std::vector<std::variant<Value, StarValue>> input) {
  inputs_.emplace_back(std::move(input));
}

void Enum::AddOutput(std::vector<Constructor> output) {
  outputs_.emplace_back(std::move(output));
}

void AST::AddMessage(Message message) {
  messages_.insert(std::make_pair(message.name_, std::move(message)));
}

void AST::AddEnum(Enum enum_) {
  enums_.insert(std::make_pair(enum_.name_, std::move(enum_)));
}

} // namespace dbuf::parser
