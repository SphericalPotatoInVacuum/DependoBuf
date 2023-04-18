#pragma once

#include "core/parser/expression.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace dbuf::parser {

struct TypedVariable {
  uint64_t name_;
  TypeExpression type_expression_;
};

struct Constructor {
  void AddField(TypedVariable field);

  uint64_t name_;
  std::vector<TypedVariable> fields_ = {};
};

struct DependentType {
  void AddDependency(TypedVariable type_dependency_);

  std::vector<TypedVariable> type_dependencies_;
};

struct Message : public DependentType {
  void AddField(TypedVariable field);

  uint64_t name_;
  std::vector<TypedVariable> fields_ = {};
};

struct Enum : public DependentType {
  void AddInput(std::vector<std::variant<Value, StarValue>> input);
  void AddOutput(std::vector<Constructor> output);

  uint64_t name_;
  std::vector<std::vector<std::variant<Value, StarValue>>> inputs_;
  std::vector<std::vector<Constructor>> outputs_;
};

struct AST {
  void AddMessage(Message message);
  void AddEnum(Enum enum_);

  std::unordered_map<uint64_t, Message> messages_;
  std::unordered_map<uint64_t, Enum> enums_;
};

} // namespace dbuf::parser
