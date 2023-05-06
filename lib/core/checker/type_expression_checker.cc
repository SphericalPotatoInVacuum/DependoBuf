#include "type_expression_checker.h"

#include "core/ast/ast.h"
#include "core/ast/expression.h"
#include "location.hh"

#include <string>
#include <vector>

namespace dbuf::checker {

TypeExpressionChecker::TypeExpressionChecker(
    ast::AST &ast,
    std::vector<InternedString> &sorted_graph)
    : ast_(ast)
    , sorted_graph_(sorted_graph) {}

void TypeExpressionChecker::GetConstructorToEnum() {
  for (const auto &ast_enum : ast_.enums) {
    for (const auto &rule : ast_enum.second.pattern_mapping) {
      for (const auto &constructor : rule.outputs) {
        constructor_to_enum_[constructor.identifier.name] = ast_enum.second.identifier.name;
      }
    }
  }
}

void TypeExpressionChecker::PushScope() {
  context_.emplace_back();
}

void TypeExpressionChecker::PopScope() {
  if (context_.empty()) {
    throw std::logic_error("Can't delete scope from empty scopes.");
  }
  context_.pop_back();
}

bool TypeExpressionChecker::IsInScope(InternedString name) {
  return std::ranges::any_of(context_.begin(), context_.end(), [&name](auto &scope) {
    return scope.contains(name);
  });
}

void TypeExpressionChecker::AddName(InternedString name, ast::TypeExpression type) {
  if (context_.empty()) {
    throw std::logic_error("Can't add name to empty scopes.");
  }

  context_.back()[name] = std::move(type);
}

bool TypeExpressionChecker::IsTypeName(InternedString name) const {
  return ast_.enums.contains(name) || ast_.messages.contains(name);
}

} // namespace dbuf::checker
