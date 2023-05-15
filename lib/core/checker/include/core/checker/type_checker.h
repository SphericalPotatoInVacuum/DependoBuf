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

protected:
  struct Z3stuff {
    explicit Z3stuff()
        : solver_(context_)
        , sorts_(
              {{InternedString("Int"), context_.int_sort()},
               {InternedString("Unsigned"), context_.int_sort()},
               {InternedString("Bool"), context_.bool_sort()},
               {InternedString("String"), context_.string_sort()}}) {}

    using NameToSort = std::unordered_map<InternedString, z3::sort>; // NameToSort[type_name] = sort
    using NameToConstructor =
        std::unordered_map<InternedString, z3::func_decl>; // NameToConstructor[cons_name] = constructor
    using FieldToAccessor = std::unordered_map<InternedString, z3::func_decl>;   // FieldToAccessor[field] = accessor
    using NameToFields    = std::unordered_map<InternedString, FieldToAccessor>; // NameToFields[type] = FieldToAccessor

    z3::context context_;
    z3::solver solver_;
    NameToSort sorts_;               // z3_sorts_[type_name] = sort
    NameToConstructor constructors_; // z3_constructors_[cons_name] = constructor
    NameToFields accessors_;         // z3_accessors_[cons_name][field_name] = accessor
  };

private:
  class Scope {
  public:
    explicit Scope(std::deque<Scope *> *ctx_ptr)
        : ctx_(*ctx_ptr) {
      ctx_.push_back(this);
      DLOG(INFO) << "Added a scope to type checker";
    }
    ~Scope() {
      assert(this == ctx_.back());
      ctx_.pop_back();
      DLOG(INFO) << "Popped a scope from type checker";
    }

    // Delete the copy constructor and copy assignment operator to prevent copying
    Scope(const Scope &)            = delete;
    Scope &operator=(const Scope &) = delete;

    void AddName(InternedString name, ast::TypeExpression type);
    [[nodiscard]] const ast::TypeExpression &LookupName(InternedString name) const;

  private:
    std::unordered_map<InternedString, ast::TypeExpression> vars_;
    std::deque<Scope *> &ctx_;
  };

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
  std::deque<Scope *> context_;
  ErrorList errors_;
  std::unordered_map<InternedString, InternedString> constructor_to_enum_;

  Z3stuff z3_stuff_;

  struct ExpressionToZ3 {
    TypeChecker::Z3stuff &z3_stuff;

    z3::expr operator()(const ast::BinaryExpression &binary_expression) {
      z3::expr left  = std::visit(*this, *binary_expression.left);
      z3::expr right = std::visit(*this, *binary_expression.right);
      switch (binary_expression.type) {
      case ast::BinaryExpressionType::Plus:
        return left + right;
      case ast::BinaryExpressionType::Minus:
        return left - right;
      case ast::BinaryExpressionType::Star:
        return left * right;
      case ast::BinaryExpressionType::Slash:
        return left / right;
      case ast::BinaryExpressionType::And:
        return left && right;
      case ast::BinaryExpressionType::Or:
        return left || right;
      default:
        throw std::runtime_error("Unknown binary expression type");
      }
    }

    z3::expr operator()(const ast::UnaryExpression &unary_expression) {
      z3::expr expr = std::visit(*this, *unary_expression.expression);
      switch (unary_expression.type) {
      case ast::UnaryExpressionType::Minus:
        return -expr;
      case ast::UnaryExpressionType::Bang:
        return !expr;
      default:
        throw std::runtime_error("Unknown unary expression type");
      }
    }

    z3::expr operator()(const ast::VarAccess & /*var_access*/) {
      throw std::runtime_error("Var access is not directly convertible to Z3 expressions");
    }

    z3::expr operator()(const ast::ScalarValue<bool> &value) {
      return z3_stuff.context_.bool_val(value.value);
    }
    z3::expr operator()(const ast::ScalarValue<int64_t> &value) {
      return z3_stuff.context_.int_val(value.value);
    }
    z3::expr operator()(const ast::ScalarValue<uint64_t> &value) {
      return z3_stuff.context_.int_val(value.value);
    }
    z3::expr operator()(const ast::ScalarValue<double> &value) {
      return z3_stuff.context_.fpa_val(value.value);
    }
    z3::expr operator()(const ast::ScalarValue<std::string> &value) {
      return z3_stuff.context_.string_val(value.value);
    }

    z3::expr operator()(const ast::Value &value) {
      return std::visit(*this, value);
    }

    z3::expr operator()(const ast::TypeExpression & /*type_expression*/) {
      throw std::runtime_error("Type expressions are not directly convertible to Z3 expressions");
    }

    z3::expr operator()(const ast::ConstructedValue &value) {
      z3::expr_vector args(z3_stuff.context_);
      for (const auto &[field_name, field] : value.fields) {
        args.push_back(std::visit(*this, *field));
      }
      return z3_stuff.constructors_.at(value.constructor_identifier.name)(args);
    }

    z3::expr operator()(const ast::Expression &expression) {
      return std::visit(*this, expression);
    }
  };
};

} // namespace dbuf::checker
