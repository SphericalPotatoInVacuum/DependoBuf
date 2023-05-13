#pragma once

#include "core/ast/ast.h"
#include "core/ast/expression.h"
#include "core/checker/common.h"
#include "core/interning/interned_string.h"
#include "core/substitutor/substitutor.h"
#include "glog/logging.h"
#include "location.hh"
#include "z3++.h"

#include <deque>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace dbuf::checker {

class TypeChecker {
public:
  explicit TypeChecker(const ast::AST &ast, const std::vector<InternedString> &sorted_graph);

  ErrorList CheckTypes();

  void operator()(const ast::Message &ast_message);
  void operator()(const ast::Enum &ast_enum);

private:
  using Scope = std::unordered_map<InternedString, ast::TypeExpression>;

  class TypeComparator {
  public:
    TypeComparator(const ast::Expression &expr, const ast::TypeExpression &expected, TypeChecker *checker)
        : expr_(expr)
        , expected_(expected)
        , checker_(*checker) {}

    void Compare();

    // Expression specifications
    std::optional<ast::TypeExpression> operator()(const ast::TypeExpression &expr);
    std::optional<ast::TypeExpression> operator()(const ast::BinaryExpression &expr);
    std::optional<ast::TypeExpression> operator()(const ast::UnaryExpression &expr);
    std::optional<ast::TypeExpression> operator()(const ast::VarAccess &expr);
    std::optional<ast::TypeExpression> operator()(const ast::Value &val);

    // Value specifications
    template <typename T>
    std::optional<ast::TypeExpression> operator()(const ast::ScalarValue<T> &val) {
      if (expected_.identifier.name != GetTypename(val)) {
        AddError(&checker_.errors_) << "Got value of type \"" << GetTypename(val) << "\", but expected type is \""
                                    << expected_.identifier.name << "\" at " << val.location;
        return {};
      }
      return {expected_};
    }

    std::optional<ast::TypeExpression> operator()(const ast::ConstructedValue &val);

  private:
    const ast::Expression &expr_;
    const ast::TypeExpression &expected_;
    TypeChecker &checker_;
  };

  /**
   * @brief Check that all dependencies are correctly defined
   *
   * @param type
   */
  void CheckDependencies(const ast::DependentType &type);
  /**
   * @brief Check that all fields are correctly defined
   *
   * @param type
   */
  void CheckFields(const ast::TypeWithFields &type);

  void CheckTypeExpression(const ast::TypeExpression &type_expression);
  bool CompareExpressions(const ast::Expression &expected, const ast::Expression &got);
  bool CompareTypeExpressions(const ast::TypeExpression &expected_type, const ast::TypeExpression &expression);

  void BuildConstructorToEnum();

  void AddName(InternedString name, ast::TypeExpression type);

  void PushScope();
  void PopScope();

  [[nodiscard]] const ast::TypeExpression &LookupName(InternedString name) const;

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

  const ast::AST &ast_;
  const std::vector<InternedString> sorted_graph_;

  Substitutor substitutor_;
  std::deque<Scope> context_;
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
