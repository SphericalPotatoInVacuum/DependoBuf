#include "ast.h"

namespace dbuf::parser {

void AST::AddMessage(std::unique_ptr<Message> &&message) {
  messages_.insert(std::make_pair(message->name_, std::move(message)));
}

Message::Message(std::string &name)
    : name_(name) {}

void Message::AddInput(std::vector<std::unique_ptr<Value>> input) {
  inputs_.emplace_back(std::move(input));
}

void Message::AddOutput(std::vector<std::unique_ptr<Constructor>> output) {
  outputs_.emplace_back(std::move(output));
}

void Message::AddDependency(std::unique_ptr<TypedVariable> type_dependency_) {}

Field::Field(std::string &name, std::unique_ptr<TypeExpression> type_expression)
    : name_(name)
    , type_expression_(std::move(type_expression)) {}

Constructor::Constructor(std::string &name)
    : name_(name) {}

void Constructor::AddField(std::unique_ptr<Field> field) { fields_.emplace_back(std::move(field)); }

} // namespace dbuf::parser
