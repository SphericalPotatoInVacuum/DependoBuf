#pragma once
#include "core/ast/ast.h"
#include "core/checker/common.h"
#include "core/checker/expression_comparator.h"

#include <optional>

namespace dbuf::checker {

class TypeComparator {
public:
  explicit TypeComparator(
      const ast::TypeExpression &expected,
      const ast::AST &ast,
      std::deque<Scope *> *context_ptr,
      Z3stuff *z3_stuff_ptr)
      : expected_(expected)
      , ast_(ast)
      , context_(*context_ptr)
      , z3_stuff_(*z3_stuff_ptr) {}

  [[nodiscard]] std::optional<Error> Compare(const ast::Expression &expr);

  // Expression specifications
  std::optional<Error> operator()(const ast::TypeExpression &expr);
  std::optional<Error> operator()(const ast::BinaryExpression &expr);
  std::optional<Error> operator()(const ast::UnaryExpression &expr);
  std::optional<Error> operator()(const ast::VarAccess &expr);
  std::optional<Error> operator()(const ast::Value &val);

  // Value specifications
  template <typename T>
  std::optional<Error> operator()(const ast::ScalarValue<T> &val) {
    if (expected_.identifier.name != GetTypename(val)) {
      return Error(
          CreateError() << "Got value of type \"" << GetTypename(val) << "\", but expected type is \""
                        << expected_.identifier.name << "\" at " << val.location);
    }
    return {};
  }

  std::optional<Error> operator()(const ast::ConstructedValue &val);

private:
  const ast::TypeExpression &expected_;
  const ast::AST &ast_;
  std::deque<Scope *> &context_;
  Z3stuff &z3_stuff_;

  [[nodiscard]] static std::optional<Error> CompareTypeExpressions(
      const ast::TypeExpression &expected_type,
      const ast::TypeExpression &expression,
      Z3stuff &z3_stuff) {
    if (expected_type.identifier.name != expression.identifier.name) {
      return Error(
          CreateError() << "Got type \"" << expression.identifier.name << "\", but expected type is \""
                        << expected_type.identifier.name << "\" at " << expression.location);
    }

    if (expected_type.parameters.size() != expression.parameters.size()) {
      return Error(
          CreateError() << "Expected " << expected_type.parameters.size() << "type parametes, but got "
                        << expression.parameters.size() << " at " << expression.location);
    }

    for (size_t id = 0; id < expected_type.parameters.size(); ++id) {
      auto error = CompareExpressions(*expected_type.parameters[id], *expression.parameters[id], z3_stuff);
      if (error) {
        return Error(
            CreateError() << "Type parameter " << id << " mismatch: " << error->message << " at "
                          << expression.location);
      }
    }

    return {};
  }

  static InternedString GetTypename(const ast::ScalarValue<bool> &) {
    return InternedString("Bool");
  }
  static InternedString GetTypename(const ast::ScalarValue<int64_t> &) {
    return InternedString("Int");
  }
  static InternedString GetTypename(const ast::ScalarValue<uint64_t> &) {
    return InternedString("Unsigned");
  }
  static InternedString GetTypename(const ast::ScalarValue<double> &) {
    return InternedString("Float");
  }
  static InternedString GetTypename(const ast::ScalarValue<std::string> &) {
    return InternedString("String");
  }
};

}; // namespace dbuf::checker
