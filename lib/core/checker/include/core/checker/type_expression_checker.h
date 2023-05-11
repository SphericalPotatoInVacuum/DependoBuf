#pragma once

#include "core/ast/ast.h"
#include "core/ast/expression.h"
#include "core/checker/common.h"
#include "core/interning/interned_string.h"
#include "core/substitutor/substitutor.h"
#include "z3++.h"

#include <deque>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace dbuf::checker {

class TypeExpressionChecker {
public:
  explicit TypeExpressionChecker(const ast::AST &ast, const std::vector<InternedString> &sorted_graph);

  void CheckTypes();

  void operator()(const ast::Message &ast_message);
  void operator()(const ast::Enum &ast_enum);

  void CheckTypeExpression(const ast::TypeExpression &type_expression);

  void CheckTypeDependencies(
      const std::vector<ast::TypedVariable> &type_dependencies,
      const std::vector<std::shared_ptr<const ast::Expression>> &type_parameters);

  void CompareExpressions(const ast::Expression &expected, const ast::Expression &got);

  void CompareTypeExpressions(const ast::TypeExpression &expected_type, const ast::TypeExpression &expression);

  // Easy case: scalar values. We just need to get the name of type and comare it with the name of
  // type expression
  template <typename T>
  void operator()(const ast::TypeExpression &expected_type, const ast::ScalarValue<T> &value) {
    if (expected_type.identifier.name != GetTypename(value)) {
      errors_.emplace_back(Error {
          .message = "Got value of type \"" + GetTypename(value).GetString() + "\", but expected type is \"" +
                     expected_type.identifier.name.GetString() + "\""});
      return;
    }
  }

  // This case should not be reachable
  // TODO(alisa-vernigor): return error
  void operator()(const ast::Expression &expected_type, const ast::Expression &star);

  void operator()(const ast::Expression & /*expected_type*/, const ast::Star & /*star*/);
  void operator()(const ast::Expression & /*expected_type*/, const ast::Value & /*value*/);

  void operator()(const ast::TypeExpression &expected_type, const ast::Star &star);

  void operator()(const ast::TypeExpression &expected_type, const ast::Value &value);

  void operator()(const ast::TypeExpression &expected_type, const ast::ConstructedValue &value);

  void operator()(const ast::TypeExpression &expected_type, const ast::VarAccess &expression);

  void operator()(const std::vector<ast::TypedVariable> &type_dependencies, const ast::TypeExpression &type_expression);

private:
  void BuildConstructorToEnum();

  [[nodiscard]] bool IsTypeName(InternedString name) const;

  void AddName(InternedString name, ast::TypeExpression type);

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
  const std::vector<InternedString> sorted_graph_;
  std::deque<std::unordered_map<InternedString, ast::TypeExpression>> context_;
  const ast::AST &ast_;
  ErrorList errors_;
  std::unordered_map<InternedString, InternedString> constructor_to_enum_;

  z3::context z3_context_;
  std::unordered_map<InternedString, z3::sort> z3_sorts_;                    // z3_sorts_[type] = sort
  std::unordered_map<InternedString, z3::func_decl> z3_constructors_;        // z3_constructors_[type] = constructor
  using FieldToAccessor = std::unordered_map<InternedString, z3::func_decl>; // FieldToAccessor[field] = accessor
  std::unordered_map<InternedString, FieldToAccessor> z3_accessors_;         // z3_accessors_[type][field] = accessor
  z3::solver z3_solver_;
};

} // namespace dbuf::checker
