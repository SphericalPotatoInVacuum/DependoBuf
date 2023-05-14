#pragma once

#include "core/ast/ast.h"
#include "core/ast/expression.h"
#include "core/interning/interned_string.h"

#include <deque>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <variant>
#include <vector>

namespace dbuf {

struct Substitutor {
  void AddSubstitution(InternedString name, const std::shared_ptr<const ast::Expression> &expression);
  void PushScope();
  void PopScope();

  template <typename T>
  ast::Expression operator()(const ast::ScalarValue<T> &value) {
    return ast::ScalarValue<T>(value);
  }

  ast::Expression operator()(const ast::BinaryExpression &expression);
  ast::Expression operator()(const ast::UnaryExpression &expression);

  ast::Expression operator()(const ast::Expression &, const ast::Expression &);

  ast::Expression operator()(const ast::Expression &, const ast::VarAccess &);
  ast::Expression operator()(const ast::VarAccess &value, const ast::ConstructedValue &substitution);
  ast::Expression operator()(const ast::VarAccess &value, const ast::VarAccess &substitution);
  template <typename T>
  ast::Expression operator()(const ast::VarAccess &value, const ast::ScalarValue<T> &substitution) {
    if (value.field_identifiers.empty()) {
      return substitution;
    }
    throw std::runtime_error("Field access on scalar value");
  }
  ast::Expression operator()(const ast::VarAccess &value, const ast::Value &substitution) {
    return std::visit(*this, ast::Expression(value), substitution);
  }
  template <typename T, typename U>
  ast::Expression operator()(const T &, const U &) {
    throw std::runtime_error("Substitution error");
  }

  ast::Expression operator()(const ast::VarAccess &value);

  ast::Expression operator()(const ast::ConstructedValue &value);

  ast::Expression operator()(const ast::Value &value);

  ast::Expression operator()(const ast::TypeExpression &type_expression);

private:
  using Scope = std::unordered_map<InternedString, std::shared_ptr<const ast::Expression>>;
  std::deque<Scope> substitute_;
};

} // namespace dbuf
