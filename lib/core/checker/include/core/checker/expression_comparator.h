#pragma once

#include "core/ast/ast.h"
#include "core/checker/common.h"
#include "glog/logging.h"
#include "z3++.h"

namespace dbuf::checker {

struct Z3stuff {
  explicit Z3stuff()
      : solver_(context_)
      , sorts_(
            {{InternedString("Int"), context_.int_sort()},
             {InternedString("Unsigned"), context_.int_sort()},
             {InternedString("Bool"), context_.bool_sort()},
             {InternedString("String"), context_.string_sort()}}) {}

  using NameToSort        = std::unordered_map<InternedString, z3::sort>;        // NameToSort[type_name] = sort
  using NameToConstructor = std::unordered_map<InternedString, z3::func_decl>;   // NameToConstructor[cons_name] = cons
  using FieldToAccessor   = std::unordered_map<InternedString, z3::func_decl>;   // FieldToAccessor[field] = accessor
  using NameToFields      = std::unordered_map<InternedString, FieldToAccessor>; // NameToFields[type] = FieldToAccessor

  z3::context context_;
  z3::solver solver_;
  NameToSort sorts_;               // z3_sorts_[type_name] = sort
  NameToConstructor constructors_; // z3_constructors_[cons_name] = constructor
  NameToFields accessors_;         // z3_accessors_[cons_name][field_name] = accessor
};

struct ExpressionToZ3 {
  Z3stuff &z3_stuff;

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

[[nodiscard]] std::optional<Error>
CompareExpressions(const ast::Expression &expected, const ast::Expression &got, Z3stuff &z3_stuff);

} // namespace dbuf::checker
