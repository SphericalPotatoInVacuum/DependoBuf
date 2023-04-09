#pragma once

#include "expression.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace dbuf::parser {

struct TypedVariable {
  std::string name;
  TypeExpression type_expression;
};

class Message {
public:
  explicit Message(std::string &name);
  void AddInput(std::vector<std::unique_ptr<Value>> input);
  void AddDependencie(std::unique_ptr<TypedVariable> type_dependency_);

  std::string name_;
  std::unordered_map<std::string, std::unique_ptr<TypedVariable>> type_dependencies_;
  std::vector<std::vector<std::unique_ptr<Value>>> inputs_;
};
class AST {
public:
  void AddMessage(std::unique_ptr<Message>);

  std::unordered_map<std::string, std::unique_ptr<Message>> messages_;
};

} // namespace dbuf::parser
