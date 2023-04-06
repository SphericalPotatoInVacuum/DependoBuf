#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace dbuf {
namespace parser {

struct AST {
  std::unordered_map<std::string, Message> messages;
};

struct Message {
  std::string name;
  std::unordered_map<std::string, TypedVariable> type_dependencies;
};

struct TypedVariable {
  std::string name;
  TypeExpression type_expression;
};

struct TypeExpression {
  std::string type_name;
  std::vector<std::shared_ptr<Expression>> type_parameters;
};

} // namespace parser
} // namespace dbuf
