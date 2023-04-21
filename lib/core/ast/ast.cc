#include "core/ast/ast.h"

#include <iostream>
#include <iterator>

namespace dbuf::ast {

void AST::AddMessage(Message &&message_arg) {
  messages_.insert(std::make_pair(message_arg.name_, std::move(message_arg)));
}

void AST::AddEnum(Enum &&enum_arg) {
  enums_.insert(std::make_pair(enum_arg.name_, std::move(enum_arg)));
}

} // namespace dbuf::ast
