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

uint64_t AST::GetInterning(std::string &&input_string) {
  return interning_.GetInterning(std::move(input_string));
}

} // namespace dbuf::ast
