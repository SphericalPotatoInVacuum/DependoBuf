#pragma once

#include "core/ast/ast.h"
#include "core/checker/common.h"
#include "core/interning/interned_string.h"

#include <deque>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace dbuf::checker {

struct NameResolutionChecker {
  void operator()(const ast::Value &value);

  void operator()(const std::vector<ast::Enum::Rule::InputPattern> &inputs);

  ErrorList operator()(const ast::AST &ast);

  void operator()(const std::unordered_map<InternedString, ast::Enum> &enums);

  void operator()(const std::vector<ast::Enum::Rule> &rules);

  void operator()(const std::vector<ast::Constructor> &constructors);

  void operator()(const ast::Star &);

  void operator()(const std::unordered_map<InternedString, ast::Message> &messages);

  void operator()(const std::vector<ast::TypedVariable> &dependencies);

  void operator()(const ast::TypedVariable &typed_variable);

  void operator()(const ast::TypeExpression &type_expression);

  void operator()(const std::vector<std::unique_ptr<ast::Expression>> &expressions);

  void operator()(const ast::ConstructedValue &value);

  void operator()(
      InternedString constructor_identifier,
      const std::vector<std::pair<InternedString, std::unique_ptr<ast::Expression>>> &fields);

  void operator()(const std::pair<InternedString, std::unique_ptr<ast::Expression>> &field);

  template <typename T>
  void operator()(const ast::ScalarValue<T> &) {}

  void operator()(const ast::BinaryExpression &expr) const;
  void operator()(const ast::UnaryExpression &expr) const;

  void operator()(const ast::VarAccess &var_access);

private:
  ErrorList errors_;

  bool isShadowing_ = false;

  std::deque<std::unordered_set<InternedString>> scopes_;
  std::unordered_map<InternedString, std::unordered_set<InternedString>> constructors_fields_;

  bool IsInScope(InternedString name);

  void AddName(InternedString name, std::string &&identifier_type);

  std::unordered_map<
      InternedString,
      std::unordered_set<InternedString>> static GetConstructorFields(const ast::AST &ast);

  void AddGlobalNames(const ast::AST &ast);

  void PushScope();

  void PopScope();
};

} // namespace dbuf::checker
