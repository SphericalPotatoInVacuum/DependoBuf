
#pragma once

#include "core/ast/ast.h"
#include "core/ast/expression.h"
#include "core/checker/common.h"
#include "core/interning/interned_string.h"
#include "core/substitutor/substitutor.h"

#include <deque>
#include <string>
#include <unordered_map>
#include <vector>

namespace dbuf::checker {

class TypeExpressionChecker {
  // template <typename T>
  //  void operator()(const ast::TypeExpression &, const T &) {}

  explicit TypeExpressionChecker(ast::AST &ast, std::vector<InternedString> &dependency_graph_);

  void operator()(const ast::TypeExpression &type_expression) {
    if (!IsTypeName(type_expression.identifier.name)) {
      errors_.emplace_back(Error {
          .message = "Unknown typename: \"" + type_expression.identifier.name.GetString() + "\""});
      return;
    }

    auto it_message = ast_.messages.find(type_expression.identifier.name);
    if (it_message != ast_.messages.end()) {
      (*this)(it_message->second.type_dependencies, type_expression);
    }

    auto it_enum = ast_.enums.find(type_expression.identifier.name);
    if (it_enum != ast_.enums.end()) {
      (*this)(it_enum->second.type_dependencies, type_expression);
    }
  }

  void operator()() {
    for (const auto &node : sorted_graph_) {
      if (ast_.messages.contains(node)) {
        auto it = ast_.messages.find(node);
        for (const auto &dependency : it->second.type_dependencies) {
          if (!ast_.messages.contains(dependency.name) && !ast_.enums.contains(dependency.name)) {
            errors_.push_back({});
            break;
          }
        }
      } else {
      }
    }
  }

  void operator()(const ast::TypeExpression &type_expression);

  void operator()(const ast::TypeExpression &expected_type, const ast::VarAccess &expression);

  void operator()(const ast::Expression &expected, const ast::Expression &got) {
    // TODO(SphericalPotatoInVacuum): compare expressions
  }

  void operator()(const ast::TypeExpression &expected_type, const ast::TypeExpression &expression);

  void operator()(const ast::TypeExpression &expected_type, const ast::ConstructedValue &value);

  template <typename T>
  void operator()(const ast::TypeExpression &expected_type, const ast::ScalarValue<T> &value) {
    if (expected_type.identifier.name != GetTypename(value)) {
      errors_.emplace_back(Error {
          .message = "Got value of type \"" + GetTypename(value).GetString() +
                     "\", but expected type is \"" + expected_type.identifier.name.GetString() +
                     "\""});
      return;
    }
  }

  void operator()(
      const std::vector<ast::TypedVariable> &type_dependencies,
      const ast::TypeExpression &type_expression);

  void operator()(const std::unordered_map<InternedString, ast::Message> &messages);
  void operator()(const std::unordered_map<InternedString, ast::Enum> &enums);

  ast::TypeExpression substitute(ast::TypeExpression type_expression) {
    for (auto &expression : type_expression.parameters) {
    }
  }

  ast::TypeExpression substitute(ast::Value type_expression) {
    for (auto &expression : type_expression.parameters) {
    }
  }

private:
  void GetConstructorToEnum();

  [[nodiscard]] bool IsTypeName(InternedString name) const;

  void AddName(InternedString name, InternedString type);

  bool IsInScope(InternedString name);

  void PushScope();

  void PopScope();

  template <typename T>
  InternedString GetTypename(const ast::ScalarValue<T> &value) {}

  template <>
  InternedString GetTypename<bool>(const ast::ScalarValue<bool> &) {
    return InternedString("Bool");
  }
  template <>
  InternedString GetTypename<int64_t>(const ast::ScalarValue<int64_t> &) {
    return InternedString("Int");
  }
  template <>
  InternedString GetTypename<uint64_t>(const ast::ScalarValue<uint64_t> &) {
    return InternedString("Unsigned");
  }
  template <>
  InternedString GetTypename<double>(const ast::ScalarValue<double> &) {
    return InternedString("Float");
  }
  template <>
  InternedString GetTypename<std::string>(const ast::ScalarValue<std::string> &) {
    return InternedString("String");
  }

  Substitutor substitutor_;
  std::vector<InternedString> sorted_graph_;
  std::deque<std::unordered_map<InternedString, ast::TypeExpression>> context_;
  ast::AST &ast_;
  ErrorList errors_;
  std::unordered_map<InternedString, InternedString> constructor_to_enum_;
};

} // namespace dbuf::checker
