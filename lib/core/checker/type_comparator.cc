/*
This file is part of DependoBuf project.

Copyright (C) 2023 Alexander Bogdanov, Alice Vernigor

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/
#include "core/checker/type_comparator.h"

#include "core/ast/ast.h"
#include "core/ast/expression.h"
#include "core/checker/common.h"
#include "core/checker/expression_comparator.h"
#include "core/interning/interned_string.h"
#include "core/substitutor/substitutor.h"
#include "glog/logging.h"
#include "location.hh"

#include <memory>
#include <optional>
#include <variant>

namespace dbuf::checker {

struct Matcher {
  Z3stuff &z3_stuff;
  std::deque<Scope *> &context;
  Substitutor &substitutor;
  const ast::AST &ast;

  bool operator()(const ast::Expression &arg, const ast::Enum::Rule::InputPattern &pattern) {
    bool matches = std::visit(*this, arg, pattern);
    DLOG(INFO) << arg << (matches ? " matches " : " does not match ") << pattern;
    return matches;
  }

  // Anything matches the star
  template <typename T>
  bool operator()(const T &arg, const ast::Star & /* star */) {
    DLOG(INFO) << "Matched " << arg << " to Star";
    return true;
  }

  // Anything matches the var access
  template <typename T>
  bool operator()(const T &arg, const ast::VarAccess &pattern) {
    DLOG(INFO) << "Matched " << arg << " to VarAccess " << pattern;
    substitutor.AddSubstitution(pattern.var_identifier.name, std::make_shared<ast::Expression>(arg));
    return true;
  }

  bool operator()(const ast::Value &arg, const ast::Value &pattern) {
    DLOG(INFO) << "Matching Value " << arg << " against Value" << pattern;
    return std::visit(*this, arg, pattern);
  }

  template <typename T>
  bool operator()(const T &arg, const ast::ConstructedValue &pattern) {
    DLOG(INFO) << "Tried to match " << arg << " against ConstructedValue " << pattern;
    return false;
  }

  template <typename T>
  bool operator()(const ast::ConstructedValue &arg, const T &pattern) {
    DLOG(INFO) << "Tried to match ConstructedValue " << arg << " against " << pattern;
    return false;
  }

  bool operator()(const ast::ConstructedValue &arg, const ast::ConstructedValue &pattern) {
    DLOG(INFO) << "Matching constructed value " << arg << " against " << pattern;
    if (arg.constructor_identifier.name != pattern.constructor_identifier.name) {
      return false;
    }
    if (arg.fields.size() != pattern.fields.size()) {
      return false;
    }
    for (size_t i = 0; i < arg.fields.size(); ++i) {
      const auto &[field_name, pattern_field] = pattern.fields[i];
      const auto &[_, arg_field]              = arg.fields[i];
      DLOG(INFO) << "Matching field " << field_name.name;
      if (!std::visit(*this, *arg_field, *pattern_field)) {
        DLOG(INFO) << *arg_field << " didn't match " << *pattern_field << " in field " << field_name.name;
        return false;
      }
    }
    return true;
  }

  bool operator()(const ast::VarAccess &arg, const ast::ConstructedValue &pattern) {
    DLOG(INFO) << "Matching VarAccess " << arg << " against ConstructedValue " << pattern;
    const auto &type_variant = ast.types.at(GetVarAccessType(arg, ast, &context).identifier.name);
    if (std::holds_alternative<ast::Enum>(type_variant)) {
      return false;
    }
    for (const auto &[field_name, field] : pattern.fields) {
      ast::VarAccess arg_field = arg;
      arg_field.field_identifiers.push_back(field_name);
      if (!(*this)(arg_field, *field)) {
        return false;
      }
    }
    return true;
  }

  template <typename T, typename U>
  bool operator()(const T &arg, const U &pattern) {
    // CompareExpressions returns std::optional<Error> if the expressions don't match
    // so if there is no value then the expressions match
    DLOG(INFO) << "Matching " << arg << " against " << pattern << " using CompareExpressions";
    return !CompareExpressions(pattern, arg, z3_stuff, ast, context).has_value();
  }
};

std::optional<Error> TypeComparator::Compare(const ast::Expression &expr) {
  auto result = std::visit(*this, expr);
  if (result) {
    DLOG(ERROR) << result->message;
  } else {
    DLOG(INFO) << "Expression " << expr << " is of expected type " << expected_;
  }
  return result;
}

std::optional<Error> TypeComparator::operator()(const ast::TypeExpression & /* expr */) {
  LOG(FATAL) << "TypeExpression should not be compared";
}

void TypeComparator::ArrayConcatenation(ast::Expression &expr, ast::BinaryExpression &result, std::optional<Error> &err) {
  if (err){
    return;
  }
  if (std::holds_alternative<ast::VarAccess>(expr)) {
    const auto *expr_ptr = std::get_if<ast::VarAccess>(&expr);
    ast::Expression size_expr;
    auto expr_type_err                                   = CheckVarAccess(*expr_ptr, size_expr, true);
    if (expr_type_err) {
      err = expr_type_err;
    }
    const std::shared_ptr<ast::Expression> size_expr_ptr = std::make_shared<ast::Expression>(size_expr);
    if (result.left == nullptr && result.right == nullptr) {
      ast::BinaryExpression buf {{parser::location()}, ast::BinaryExpressionType::Plus, size_expr_ptr, nullptr};
      result = buf;
    } else if (result.right == nullptr) {
      const std::shared_ptr<ast::Expression> result_left_ptr = std::make_shared<ast::Expression>(*result.left);
      ast::BinaryExpression buf {{parser::location()}, ast::BinaryExpressionType::Plus, result_left_ptr, size_expr_ptr};
      result = buf;
    } else {
      const std::shared_ptr<ast::Expression> result_ptr = std::make_shared<ast::Expression>(result);
      ast::BinaryExpression buf {{parser::location()}, ast::BinaryExpressionType::Plus, result_ptr, size_expr_ptr};
      result = buf;
    }
    return;
  }
  if (std::holds_alternative<ast::BinaryExpression>(expr)) {
    auto bin_expr      = std::get<ast::BinaryExpression>(expr);
    auto bin_expr_left = *bin_expr.left;
    ArrayConcatenation(bin_expr_left, result,err);
    auto bin_expr_right = *bin_expr.right;
    ArrayConcatenation(bin_expr_right, result,err);
  }
}

std::optional<Error> TypeComparator::operator()(const ast::BinaryExpression &expr) {
  DLOG(INFO) << "Checking binary expression " << expr;
  std::optional<Error> left_type_err;
  std::optional<Error> right_type_err;
  std::optional<Error> size_err;
  if (expr.type == ast::BinaryExpressionType::DoubleOr) {
    ast::BinaryExpression result;
    ast::Expression buf = expr;
    std::optional<Error> concat_err;
    ArrayConcatenation(buf, result, concat_err);
    if (concat_err) {
      return concat_err;
    }
    DLOG(INFO) << *expected_.parameters[1];
    size_err = CompareExpressions(*expected_.parameters[1], result, z3_stuff_, ast_, context_);
  } else {
    left_type_err  = std::visit(*this, *expr.left);
    right_type_err = std::visit(*this, *expr.right);
  }
  if (left_type_err) {
    return left_type_err;
  }
  if (right_type_err) {
    return right_type_err;
  }
  if (size_err) {
    return size_err;
  }
  DLOG(INFO) << "Expression " << expr << " should be of type " << expected_;
  if (expr.type == ast::BinaryExpressionType::Plus) {
    if (expected_.identifier.name == InternedString("Int") || expected_.identifier.name == InternedString("Unsigned") ||
        expected_.identifier.name == InternedString("String") || expected_.identifier.name == InternedString("Float")) {
      return {};
    }
  } else if (expr.type == ast::BinaryExpressionType::Star) {
    if (expected_.identifier.name == InternedString("Int") || expected_.identifier.name == InternedString("Unsigned") ||
        expected_.identifier.name == InternedString("Float")) {
      return {};
    }
  } else if (expr.type == ast::BinaryExpressionType::Minus) {
    if (expected_.identifier.name == InternedString("Int") || expected_.identifier.name == InternedString("Float")) {
      return {};
    }
  } else if (expr.type == ast::BinaryExpressionType::Slash) {
    if (expected_.identifier.name == InternedString("Float")) {
      return {};
    }
  } else if (
      expr.type == ast::BinaryExpressionType::And || expr.type == ast::BinaryExpressionType::Or ||
      expr.type == ast::BinaryExpressionType::In) {
    if (expected_.identifier.name == InternedString("Bool")) {
      return {};
    }
  } else if (expr.type == ast::BinaryExpressionType::DoubleAnd || expr.type == ast::BinaryExpressionType::BackSlash) {
    if (expected_.identifier.name == InternedString("Set")) {
      return {};
    }
  } else if (expr.type == ast::BinaryExpressionType::DoubleOr) {
    if (expected_.identifier.name == InternedString("Array") || expected_.identifier.name == InternedString("Set")) {
      return {};
    }
  }
  DLOG(ERROR) << "Invalid operator use in expression " << expr;
  return Error(
      CreateError() << "Operator \"" << static_cast<char>(expr.type) << "\""
                    << " is not supported by type " << expected_.identifier.name << "\" at " << expr.location);
  return {};
}

std::optional<Error> TypeComparator::operator()(const ast::UnaryExpression &expr) {
  DLOG(INFO) << "Checking unary expression " << expr;
  auto expr_err = std::visit(*this, *expr.expression);
  if (expr_err) {
    return expr_err;
  }
  if (expr.type == ast::UnaryExpressionType::Minus) {
    if (expected_.identifier.name == InternedString("Int") || expected_.identifier.name == InternedString("Float")) {
      return {};
    }
  }
  if (expr.type == ast::UnaryExpressionType::Bang) {
    if (expected_.identifier.name == InternedString("Bool")) {
      return {};
    }
  }
  DLOG(ERROR) << "Invalid operator use in expression " << expr;
  return Error(
      CreateError() << "Operator \"" << static_cast<char>(expr.type) << "\""
                    << " is not supported by type " << expected_.identifier.name << "\" at " << expr.location);
  return {};
}

std::optional<Error> TypeComparator::operator()(const ast::VarAccess &expr) {
  ast::Expression buf = ast::Expression();
  return CheckVarAccess(expr, buf);
};

std::optional<Error> TypeComparator::operator()(const ast::Value &val) {
  return std::visit(*this, val);
}

std::optional<Error> TypeComparator::operator()(const ast::ConstructedValue &val) {
  const InternedString &type_name = ast_.constructor_to_type.at(val.constructor_identifier.name);
  if (type_name != expected_.identifier.name) {
    DLOG(ERROR) << "Invalid type of constructed value " << val << " expected " << expected_.identifier.name << " got "
                << type_name << " at " << val.location;
    return Error(
        CreateError() << "Got value of type \"" << type_name << "\", but expected type is \""
                      << expected_.identifier.name << "\" at " << val.location);
  }

  if (std::holds_alternative<ast::Message>(ast_.types.at(type_name))) {
    DLOG(INFO) << "Constructed value " << val << " has a message type " << type_name << ", checking fields";
    const auto &ast_message = std::get<ast::Message>(ast_.types.at(type_name));
    return CheckConstructedValue(val, ast_message);
  }
  DLOG(INFO) << "Constructed value " << val << " has an enum type " << type_name
             << ", checking constructor availability";
  const auto &ast_enum = std::get<ast::Enum>(ast_.types.at(type_name));
  DLOG(INFO) << "Matching expression " << expected_ << " against enum " << type_name;
  for (const auto &rule : ast_enum.pattern_mapping) {
    DLOG(INFO) << "Trying input patterns " << rule.inputs;
    bool matches = true;
    substitutor_.PushScope();
    for (size_t i = 0; i < rule.inputs.size(); ++i) {
      if (!Matcher {z3_stuff_, context_, substitutor_, ast_}(*expected_.parameters[i], rule.inputs[i])) {
        matches = false;
        break;
      }
      substitutor_.AddSubstitution(ast_enum.type_dependencies[i].name, expected_.parameters[i]);
    }
    if (!matches) {
      substitutor_.PopScope();
      continue;
    }
    for (const auto &constructor : rule.outputs) {
      if (constructor.identifier.name != val.constructor_identifier.name) {
        continue;
      }
      DLOG(INFO) << "Found constructor " << constructor.identifier.name << ", checking fields";
      return CheckConstructedValue(val, constructor);
    }
  }
  return Error(
      CreateError() << "Constructor \"" << val.constructor_identifier.name << "\" cannot be used in this context at "
                    << val.location);
}

std::optional<Error> TypeComparator::operator()(const ast::CollectionValue &) {
  return {};
}

[[nodiscard]] std::optional<Error> TypeComparator::CompareTypeExpressions(
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
  if (expected_type.identifier.name != InternedString("Array") &&
      expected_type.identifier.name != InternedString("Set")) {
    for (size_t id = 0; id < expected_type.parameters.size(); ++id) {
      auto error =
          CompareExpressions(*expected_type.parameters[id], *expression.parameters[id], z3_stuff, ast_, context_);
      if (error) {
        return Error(
            CreateError() << "Type parameter " << id << " mismatch: " << error->message << " at "
                          << expression.location);
      }
    }
  } else {
    const auto *expression_collection_type_ptr = std::get_if<ast::TypeExpression>(&*expression.parameters[0]);
    const auto *expected_collection_type_ptr   = std::get_if<ast::TypeExpression>(&*expected_type.parameters[0]);
    if (expression_collection_type_ptr != nullptr && expected_collection_type_ptr != nullptr) {
      if (expression_collection_type_ptr->identifier.name != expected_collection_type_ptr->identifier.name) {
        return Error(
            CreateError() << "Expected " << expected_type.identifier.name << " of type "
                          << expected_collection_type_ptr->identifier.name << " but got "
                          << expression_collection_type_ptr->identifier.name);
      }
    } else {
      return Error(CreateError() << *expression.parameters[0] << " is unexpected type for Array");
    }
    if (expected_type.identifier.name == InternedString("Array")) {
      auto error =
          CompareExpressions(*expected_type.parameters[1], *expression.parameters[1], z3_stuff, ast_, context_);
      if (error) {
        return Error(CreateError() << "Array size mismatch: " << error->message << " at " << expression.location);
      }
    }
  }
  return {};
}

std::optional<Error> TypeComparator::CompareArrayForConcatenation(
    const ast::TypeExpression &expected_type,
    const ast::TypeExpression &expression,
    ast::Expression &size_of_array) {
  if (expected_type.identifier.name != expression.identifier.name) {
    return Error(
        CreateError() << "Got type \"" << expression.identifier.name << "\", but expected type is \""
                      << expected_type.identifier.name << "\" at " << expression.location);
  }
  if (expected_type.parameters.size() != expression.parameters.size()) {
    return Error(
        CreateError() << "Expected " << expected_type.parameters.size() << "type parametes, but got "
                      << expression.parameters.size() << " at " << expression.location);
    return {};
  }
  const auto *expression_collection_type_ptr = std::get_if<ast::TypeExpression>(&*expression.parameters[0]);
  const auto *expected_collection_type_ptr   = std::get_if<ast::TypeExpression>(&*expected_type.parameters[0]);
  if (expression_collection_type_ptr != nullptr && expected_collection_type_ptr != nullptr) {
    if (expression_collection_type_ptr->identifier.name != expected_collection_type_ptr->identifier.name) {
      return Error(
          CreateError() << "Expected " << expected_type.identifier.name << " of type "
                        << expected_collection_type_ptr->identifier.name << " but got "
                        << expression_collection_type_ptr->identifier.name);
    }
  } else {
    return Error(CreateError() << *expression.parameters[0] << " is unexpected type for Array");
  }
  size_of_array = *expression.parameters[1];
  return {};
}

std::optional<Error>
TypeComparator::CheckVarAccess(const ast::VarAccess &expr, ast::Expression &size_of_array, bool flag_for_array_check) {
  const Scope &outer_scope = *context_.back();
  DLOG(INFO) << "Checking var access: " << expr;
  if (expr.field_identifiers.empty()) {
    if (!flag_for_array_check) {
      return CompareTypeExpressions(expected_, outer_scope.LookupName(expr.var_identifier.name), z3_stuff_);
    }
    return CompareArrayForConcatenation(expected_, outer_scope.LookupName(expr.var_identifier.name), size_of_array);
  }
  InternedString message_name   = outer_scope.LookupName(expr.var_identifier.name).identifier.name;
  InternedString expected_field = expr.field_identifiers[0].name;
  const auto &type              = ast_.types.at(message_name);
  if (std::holds_alternative<ast::Enum>(type)) {
    return Error(CreateError() << "Field access works only for messages, but \"" << message_name << "\" is enum");
  }
  const auto &message = std::get<ast::Message>(type);
  bool found          = false;

  Scope scope(&context_);
  for (const auto &dependencie : message.type_dependencies) {
    scope.AddName(dependencie.name, dependencie.type_expression);
  }
  for (const auto &field : message.fields) {
    scope.AddName(field.name, field.type_expression);
    if (field.name == expected_field) {
      found = true;
      break;
    }
  }
  if (!found) {
    return Error(CreateError() << "Field \"" << expected_field << "\" not found in message \"" << message_name << "\"");
  }
  ast::VarAccess var_access {
      {{parser::location()}, {expected_field}},
      std::vector<ast::Identifier>(expr.field_identifiers.begin() + 1, expr.field_identifiers.end())};
  return CheckVarAccess(var_access, size_of_array, flag_for_array_check);
}

std::optional<Error>
TypeComparator::CheckConstructedValue(const ast::ConstructedValue &val, const ast::TypeWithFields &constructor) {
  for (size_t i = 0; i < constructor.fields.size(); ++i) {
    const auto &field         = constructor.fields[i];
    const auto &expected_type = std::get<ast::TypeExpression>(substitutor_(field.type_expression));
    DLOG(INFO) << "Checking that field " << field.name << " has type " << expected_type;
    if (std::holds_alternative<ast::VarAccess>(*val.fields[i].second)) {
      const auto &var_access = std::get<ast::VarAccess>(*val.fields[i].second);
      context_.back()->AddName(var_access.var_identifier.name, expected_type);
      continue;
    }
    auto field_err =
        TypeComparator(expected_type, ast_, &context_, &substitutor_, &z3_stuff_).Compare(*val.fields[i].second);
    if (field_err) {
      DLOG(ERROR) << "Field " << field.name << " has incorrect type";
      return field_err;
    }
  }
  return {};
}

} // namespace dbuf::checker
