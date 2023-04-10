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

void Message::AddDependency(std::unique_ptr<TypedVariable> type_dependency_) {}

} // namespace dbuf::parser
