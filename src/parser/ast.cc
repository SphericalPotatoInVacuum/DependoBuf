#include "parser/ast.h"

#include <iostream>
#include <iterator>

namespace dbuf::parser {

void Constructor::AddField(TypedVariable& field) {
  fields_.push_back(field);
}

void DependentType::AddDependency(TypedVariable& type_dependency_) {
  type_dependencies_.push_back(type_dependency_);
  iterators_.insert(std::make_pair(type_dependency_.name_, std::prev(type_dependencies_.cend())));
}

void Message::AddField(TypedVariable? field) {
  fields_.push_back(field);
}

void Enum::AddInput(std::vector<std::variant<Value, StarValue>>& input) {
  inputs_.emplace_back(input);
}

void Enum::AddOutput(std::vector<Constructor>& output) {
  outputs_.emplace_back(output);
}

void AST::AddMessage(Message& message) {
  messages_.insert(std::make_pair(message.name_, message));
}

void AST::AddEnum(Enum& enum_) {
  enums_.insert(std::make_pair(enum_.name_, enum_));
}

} // namespace dbuf::parser
