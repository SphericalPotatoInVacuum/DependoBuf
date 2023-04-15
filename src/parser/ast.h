#pragma once

#include "parser/expression.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace dbuf::parser {

struct TypedVariable {
  TypedVariable() = default;
  TypedVariable(std::string name, TypeExpression type_expression);

  std::string name_;
  TypeExpression type_expression_;
};

struct Constructor {
  Constructor() = default;
  explicit Constructor(std::string name);

  void AddField(TypedVariable field);

  std::string name_;
  std::vector<TypedVariable> fields_ = {};
};

struct Message {
  Message() = default;
  explicit Message(std::string name);

  void AddDependency(TypedVariable type_dependency_);
  void AddField(TypedVariable field);

  std::string name_;
  std::unordered_map<std::string, TypedVariable> type_dependencies_;
  std::vector<TypedVariable> fields_ = {};
};

struct Enum {
  Enum() = default;
  explicit Enum(std::string name);

  void AddDependency(TypedVariable type_dependency_);

  void AddInput(std::vector<std::unique_ptr<std::variant<Value, StarValue>>> input);
  void AddOutput(std::vector<Constructor> output);

  std::string name_;
  std::unordered_map<std::string, TypedVariable> type_dependencies_;
  std::vector<std::vector<std::unique_ptr<std::variant<Value, StarValue>>>> inputs_;
  std::vector<std::vector<Constructor>> outputs_;
};

struct AST {
  void AddMessage(Message message);
  void AddEnum(Enum enum_);

  std::unordered_map<std::string, Message> messages_;
  std::unordered_map<std::string, Enum> enums_;
};

} // namespace dbuf::parser
