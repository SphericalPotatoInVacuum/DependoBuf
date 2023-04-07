#include "ast.h"

namespace dbuf {
namespace parser {

    void AST::AddMessage(std::unique_ptr<Message> message) {
        messages_[message->name_] = std::move(message);
    }

    Message::Message(std::string& name): name_(name) {}

    void Message::AddInput(std::vector<std::unique_ptr<Value>> input) {
        inputs_.emplace_back(std::move(input));
    }

    void Message::AddDependencie(std::unique_ptr<TypedVariable> type_dependency_){
    
    }

} // namespace parser
} // namespace dbuf