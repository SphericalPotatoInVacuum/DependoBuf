#pragma once

#include "core/ast/ast.h"
#include "core/ast/expression.h"
#include "core/interning/interned_string.h"

#include <memory>
#include <unordered_map>
#include <variant>
#include <vector>

namespace dbuf {

struct Substitutor {
  void AddSubstitution(InternedString name, const std::shared_ptr<const ast::Expression> &expression);
  void ClearSubstitutionMap();

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

  ast::Expression operator()(const ast::VarAccess &value);

  ast::Expression operator()(const ast::ConstructedValue &value);

  ast::Expression operator()(const ast::Value &value);

  ast::Expression operator()(const ast::TypeExpression &type_expression);

private:
  std::unordered_map<InternedString, std::shared_ptr<const ast::Expression>> substitute_;
};

} // namespace dbuf
