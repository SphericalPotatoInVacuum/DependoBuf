#pragma once

#include "expression.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace dbuf::parser {

struct TypedVariable {
  TypedVariable(std::string name_, std::unique_ptr<TypeExpression> type_expression)
      : name(std::move(name_))
      , type_expression(std::move(type_expression)) {}
  std::string name;
  std::unique_ptr<TypeExpression> type_expression;
};

struct Field {
  explicit Field(std::string &name, std::unique_ptr<TypeExpression> type_expression);

  std::string name_;
  std::unique_ptr<TypeExpression> type_expression_;
};

struct Constructor {
  explicit Constructor(std::string &name);

  void AddField(std::unique_ptr<Field> field);

  std::string name_;
  std::vector<std::unique_ptr<Field>> fields_;
};

struct Message {
  explicit Message(std::string &name);
  void AddInput(std::vector<std::unique_ptr<Value>> input);
  void AddDependency(std::unique_ptr<TypedVariable> type_dependency_);
  void AddOutput(std::vector<std::unique_ptr<Constructor>> output);

  std::string name_;
  std::unordered_map<std::string, std::unique_ptr<TypedVariable>> type_dependencies_;
  std::vector<std::vector<std::unique_ptr<Value>>> inputs_;
  std::vector<std::vector<std::unique_ptr<Constructor>>> outputs_;
};

struct AST {
  void AddMessage(std::unique_ptr<Message> &&message);

  std::unordered_map<std::string, std::unique_ptr<Message>> messages_;
};

} // namespace dbuf::parser
