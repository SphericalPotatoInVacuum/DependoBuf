#pragma once

#include "parser/expression.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace dbuf::parser {

struct TypedVariable {
  std::string name_;
  TypeExpression type_expression_;
};

struct Constructor {
  void AddField(TypedVariable field);

  std::string name_;
  std::vector<TypedVariable> fields_ = {};
};

struct DependentType {
  void AddDependency(TypedVariable type_dependency_);

  std::vector<TypedVariable> type_dependencies_;
  std::unordered_map<std::string, std::vector<TypedVariable>::const_iterator> iterators_;
};

struct Message : public DependentType {
  void AddField(TypedVariable field);

  std::string name_;
  std::vector<TypedVariable> fields_ = {};
};

struct Enum : public DependentType {
  void AddInput(std::vector<std::variant<Value, StarValue>> input);
  void AddOutput(std::vector<Constructor> output);

  std::string name_;
  std::vector<std::vector<std::variant<Value, StarValue>>> inputs_;
  std::vector<std::vector<Constructor>> outputs_;
};

struct AST {
  void AddMessage(Message message);
  void AddEnum(Enum enum_);

  std::unordered_map<std::string, Message> messages_;
  std::unordered_map<std::string, Enum> enums_;
};

} // namespace dbuf::parser
