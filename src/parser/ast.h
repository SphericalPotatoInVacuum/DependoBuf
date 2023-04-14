#pragma once

#include "parser/expression.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace dbuf::parser {

struct TypedVariable {
  TypedVariable(std::string name_, std::unique_ptr<TypeExpression> type_expression);
  std::string name;
  std::unique_ptr<TypeExpression> type_expression;
};

struct Constructor {
  Constructor() = default;
  explicit Constructor(std::string name);

  void AddField(std::unique_ptr<TypedVariable> field);

  std::string name_;
  std::vector<std::unique_ptr<TypedVariable>> fields_ = {};
};

struct Message {
  Message() = default;
  explicit Message(std::string name);

  void AddDependency(std::unique_ptr<TypedVariable> type_dependency_);
  void AddField(std::unique_ptr<TypedVariable> field);

  std::string name_;
  std::unordered_map<std::string, std::unique_ptr<TypedVariable>> type_dependencies_;
  std::vector<std::unique_ptr<TypedVariable>> fields_ = {};
};

struct Enum {
  Enum() = default;
  explicit Enum(std::string &name);

  void AddDependency(std::unique_ptr<TypedVariable> type_dependency_);

  void AddInput(std::vector<std::unique_ptr<std::variant<Value, StarValue>>> input);
  void AddOutput(std::vector<std::unique_ptr<Constructor>> output);

  std::string name_;
  std::unordered_map<std::string, std::unique_ptr<TypedVariable>> type_dependencies_;
  std::vector<std::vector<std::unique_ptr<std::variant<Value, StarValue>>>> inputs_;
  std::vector<std::vector<std::unique_ptr<Constructor>>> outputs_;
};

struct AST {
  void AddMessage(std::unique_ptr<Message> message);
  void AddEnum(std::unique_ptr<Enum> enum_);

  std::unordered_map<std::string, std::unique_ptr<Message>> messages_;
  std::unordered_map<std::string, std::unique_ptr<Enum>> enums_;
};

} // namespace dbuf::parser
