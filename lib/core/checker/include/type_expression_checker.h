
#pragma once

#include "core/ast/ast.h"
#include "core/ast/expression.h"
#include "core/checker/common.h"
#include "core/interning/interned_string.h"

#include <deque>
#include <string>
#include <unordered_map>
#include <vector>

namespace dbuf::checker {

class TypeExpressionChecker {
  template <typename T>
  void operator()(const ast::TypeExpression &, const T &) {}

  explicit TypeExpressionChecker(ast::AST &ast);

  void operator()();

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

  std::deque<std::unordered_map<InternedString, InternedString>> scopes_;
  ast::AST &ast_;
  ErrorList errors_;
  std::unordered_map<InternedString, InternedString> constructor_to_enum_;
};

} // namespace dbuf::checker
