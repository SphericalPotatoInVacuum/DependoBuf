#pragma once

#include <memory>
#include <parser/expression.h>
#include <string>
#include <unordered_map>
#include <vector>

namespace dbuf::parser {

struct TypedVariable {
  TypedVariable(std::string name_, std::unique_ptr<TypeExpression> type_expression_)
      : name(std::move(name_))
      , type_expression(std::move(type_expression_)) {}
  std::string name;
  std::unique_ptr<TypeExpression> type_expression;
};

class Message {
public:
  explicit Message(std::string &name);
  void AddInput(std::vector<std::unique_ptr<Value>> input);
  void AddDependency(std::unique_ptr<TypedVariable> type_dependency_);

  std::string name_;
  std::unordered_map<std::string, std::unique_ptr<TypedVariable>> type_dependencies_;
  std::vector<std::vector<std::unique_ptr<Value>>> inputs_;
};

class AST {
public:
  void AddMessage(std::unique_ptr<Message> &&message);

  std::unordered_map<std::string, std::unique_ptr<Message>> messages_;
};

} // namespace dbuf::parser
