/*
This file is part of DependoBuf project.

Copyright (C) 2023 Alexander Bogdanov, Alice Vernigor

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/
#include "core/checker/type_checker.h"

#include "core/ast/ast.h"
#include "core/ast/expression.h"
#include "core/checker/common.h"
#include "core/checker/expression_comparator.h"
#include "core/checker/type_comparator.h"
#include "core/interning/interned_string.h"
#include "glog/logging.h"
#include "location.hh"
#include "z3++.h"

#include <cstddef>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

namespace dbuf::checker {

TypeChecker::TypeChecker(const ast::AST &ast, const std::vector<InternedString> &sorted_graph)
    : ast_(ast)
    , sorted_graph_(sorted_graph) {}

std::optional<Error> TypeChecker::CheckTypes() {
  for (const auto &node : sorted_graph_) {
    if (!std::visit(*this, ast_.types.at(node))) {
      return error_;
    }
  }
  return {};
}

bool TypeChecker::operator()(const ast::Message &ast_message) {
  DLOG(INFO) << "Checking message: " << ast_message.identifier.name;

  auto scope = Scope(&context_);
  if (!CheckDependencies(ast_message)) {
    return false;
  }
  if (!CheckFields(ast_message)) {
    return false;
  }

  // initializing everything, that is used for the constructor
  z3::constructors cs(z3_stuff_.context_);
  std::vector<z3::symbol> accessor_names;
  std::vector<z3::sort> accessor_sorts;
  for (const auto &field : ast_message.fields) {
    accessor_names.push_back(z3_stuff_.context_.str_symbol(field.name.GetString().c_str()));
    if (field.type_expression.identifier.name.GetString() == "Array" || field.type_expression.identifier.name.GetString() == "Set") {
      const auto type_expr = std::get<ast::TypeExpression>(*field.type_expression.parameters[0]);
      z3::sort collections_sort = GetZ3SortForCollcections(type_expr);
      accessor_sorts.push_back(collections_sort);
    } else {
      accessor_sorts.push_back(z3_stuff_.sorts_.at(field.type_expression.identifier.name));
    }
  }

  // Creating a z3 datatype for this message
  z3::symbol name_symbol = z3_stuff_.context_.str_symbol(ast_message.identifier.name.GetString().c_str());
  cs.add(name_symbol, z3_stuff_.context_.str_symbol(("is_" + ast_message.identifier.name.GetString()).c_str()), ast_message.fields.size(), accessor_names.data(), accessor_sorts.data());
  z3_stuff_.sorts_.try_emplace(ast_message.identifier.name, z3_stuff_.context_.datatype(name_symbol, cs));
 
  // Declare a constructor and accessors for this message
  z3::func_decl message_constructor(z3_stuff_.context_);
  z3::func_decl is_message_constructor_recognizer(z3_stuff_.context_);
  z3::func_decl_vector field_accessors(z3_stuff_.context_);

  // Query them from the constructors
  cs.query(0, message_constructor, is_message_constructor_recognizer, field_accessors);

  z3_stuff_.constructors_.try_emplace(ast_message.identifier.name, message_constructor);
  z3_stuff_.accessors_.try_emplace(ast_message.identifier.name, Z3stuff::FieldToFuncDecl());
  for (size_t i = 0; i < ast_message.fields.size(); ++i) {
    z3_stuff_.accessors_.at(ast_message.identifier.name).try_emplace(ast_message.fields[i].name, field_accessors[i]);
  }

  DLOG(INFO) << "Finished checking message: " << ast_message.identifier.name;
  return true;
}

bool TypeChecker::operator()(const ast::Enum &ast_enum) {
  DLOG(INFO) << "Checking enum: " << ast_enum.identifier.name;

  auto scope = Scope(&context_);
  if (!CheckDependencies(ast_enum)) {
    return false;
  }

  // The next step is to check the pattern mathcing
  for (const auto &rule : ast_enum.pattern_mapping) {
    DLOG(INFO) << "Checking rule with inputs " << rule.inputs;
    
    if (rule.inputs.size() != ast_enum.type_dependencies.size()) {
      error_ = Error(
          CreateError() << "Expected " << ast_enum.type_dependencies.size() << " inputs in pattern for enum \""
                        << ast_enum.identifier.name << "\", but got " << rule.inputs.size());
      return false;
    }

    for (const auto &constructor : rule.outputs) {
      if (!CheckFields(constructor)) {
        return false;
      }
    }
  }

  // Create a z3 sort for this enum

  z3::constructors cs(z3_stuff_.context_);
  z3::symbol name_symbol = z3_stuff_.context_.str_symbol(ast_enum.identifier.name.GetString().c_str());
  z3_stuff_.sorts_.emplace(ast_enum.identifier.name, z3_stuff_.context_.datatype_sort(name_symbol));
  DLOG(INFO) << ast_enum.identifier.name;

  for (const auto &rule : ast_enum.pattern_mapping) {
    for (const auto &constructor : rule.outputs) {
      std::vector<z3::symbol> accessor_names;
      std::vector<z3::sort> accessor_sorts;
      for (const auto &field : constructor.fields) {
        accessor_names.push_back(z3_stuff_.context_.str_symbol(field.name.GetString().c_str()));
        if (field.type_expression.identifier.name.GetString() == "Array" || field.type_expression.identifier.name.GetString() == "Set") {
          const auto type_expr = std::get<ast::TypeExpression>(*field.type_expression.parameters[0]);
          z3::sort collections_sort = GetZ3SortForCollcections(type_expr);
          accessor_sorts.push_back(collections_sort);
        } else {
          accessor_sorts.push_back(z3_stuff_.sorts_.at(field.type_expression.identifier.name));
        }
      }

      cs.add(
          z3_stuff_.context_.str_symbol(constructor.identifier.name.GetString().c_str()),
          z3_stuff_.context_.str_symbol(("is_" + constructor.identifier.name.GetString()).c_str()),
          accessor_names.size(),
          accessor_names.data(),
          accessor_sorts.data());
    }
  }

  z3_stuff_.sorts_.at(ast_enum.identifier.name) = z3_stuff_.context_.datatype(name_symbol, cs);
  DLOG(INFO) << "Created enum datatype: " << z3_stuff_.sorts_.at(ast_enum.identifier.name);

  size_t constructor_idx = 0;
  for (const auto &rule : ast_enum.pattern_mapping) {
    for (const auto &constructor : rule.outputs) {
      z3::func_decl z3_constructor(z3_stuff_.context_);
      z3::func_decl is_z3_constructor_recognizer(z3_stuff_.context_);
      z3::func_decl_vector field_accessors(z3_stuff_.context_);

      cs.query(constructor_idx, z3_constructor, is_z3_constructor_recognizer, field_accessors);

      z3_stuff_.constructors_.emplace(constructor.identifier.name, z3_constructor);
      DLOG(INFO) << "Constructor \"" << constructor.identifier.name << "\": " << z3_constructor;

      z3_stuff_.accessors_.emplace(constructor.identifier.name, Z3stuff::FieldToFuncDecl());
      for (size_t i = 0; i < constructor.fields.size(); ++i) {
        z3_stuff_.accessors_.at(constructor.identifier.name).emplace(constructor.fields[i].name, field_accessors[i]);
        DLOG(INFO) << "Accessor for field " << constructor.fields[i].name << ": " << field_accessors[i];
      }

      ++constructor_idx;
    }
  }

  DLOG(INFO) << "Finished checking enum: " << ast_enum.identifier.name;
  return true;
}

// Checks that all dependencies are correctly defined
bool TypeChecker::CheckDependencies(const ast::DependentType &type) {
  Scope &scope = *context_.back();
  for (const auto &dependency : type.type_dependencies) {
    DLOG(INFO) << "Checking dependency: " << dependency;

    if (!CheckTypeExpression(dependency.type_expression)) {
      return false;
    }
    scope.AddName(dependency.name, dependency.type_expression);
  }
  return true;
}

// Checks that all fields are correctly defined
bool TypeChecker::CheckFields(const ast::TypeWithFields &type) {
  Scope scope(&context_);
  for (const auto &field : type.fields) {
    DLOG(INFO) << "Checking field: " << field.name << " of type " << field.type_expression;
    auto after_substitution = std::get<ast::TypeExpression>(substitutor_(field.type_expression));
    DLOG(INFO) << "After substitution: " << after_substitution;

    if (!CheckTypeExpression(after_substitution)) {
      return false;
    }
    scope.AddName(field.name, after_substitution);
  }
  return true;
}

// Checks that TypeExpression is correctly defined
bool TypeChecker::CheckTypeExpression(const ast::TypeExpression &type_expression) {
  if (type_expression.identifier.name == InternedString("Int") ||
      type_expression.identifier.name == InternedString("Unsigned") ||
      type_expression.identifier.name == InternedString("Bool") ||
      type_expression.identifier.name == InternedString("String") ||
      type_expression.identifier.name == InternedString("Float")) {
    return true;
  }

  if (type_expression.identifier.name == InternedString("Array") || type_expression.identifier.name == InternedString("Set")) {
    return CheckCollectionParameters(type_expression);
  }

  const ast::DependentType &type = std::visit(
      [](const auto &type) { return static_cast<ast::DependentType>(type); },
      ast_.types.at(type_expression.identifier.name));
  if (type.type_dependencies.size() != type_expression.parameters.size()) {
    error_ = Error(
        CreateError() << "Expected " << type.type_dependencies.size() << " parameters in "
                      << type_expression.identifier.name << ", but got " << type_expression.parameters.size() << " at "
                      << type_expression.location);
      return false;
  }
  substitutor_.PushScope();

  for (size_t id = 0; id < type.type_dependencies.size(); ++id) {
    DLOG(INFO) << "Comparing parameter " << *type_expression.parameters[id] << " with type "
               << type.type_dependencies[id].type_expression;
    DLOG(INFO) << "Type before substitution: " << type.type_dependencies[id].type_expression;
    ast::TypeExpression substituted_type =
        std::get<ast::TypeExpression>(substitutor_(type.type_dependencies[id].type_expression));
    DLOG(INFO) << "Type after substitution: " << substituted_type;

    error_ = TypeComparator(substituted_type, ast_, &context_, &substitutor_, &z3_stuff_)
                        .Compare(*type_expression.parameters[id]);
    if (error_.has_value()) {
      return false;
    }
    substitutor_.AddSubstitution(type.type_dependencies[id].name, type_expression.parameters[id]);
  }

  substitutor_.PopScope();
  return true;
}

bool TypeChecker::CheckCollectionParameters(const ast::TypeExpression &type_expr) {
  if (type_expr.identifier.name == InternedString("Array")) {
    if (type_expr.parameters.size() != 2) {
      error_ = Error(CreateError() << "Expected 2 parmeters in Array constructor, but got " << type_expr.parameters.size() << " at " << type_expr.location);
      return false;
    }
    if (!std::holds_alternative<ast::TypeExpression>(*type_expr.parameters[0])) {
      error_ = Error(CreateError() << "Expected first parmeter of Array to be a TypeExpression, but got " << *type_expr.parameters[0] << " at " << type_expr.location);
      return false;
    }
    auto elem_type_expr = std::get<ast::TypeExpression>(*type_expr.parameters[0]);
    if (elem_type_expr.identifier.name.GetString() == "Array" || elem_type_expr.identifier.name.GetString() == "Set") {
      if (!CheckTypeExpression(elem_type_expr)) {
        return false;
      }
    }
    auto expected_unsigned_type_expr = ast::TypeExpression(
        {parser::location()},
        ast::Identifier({type_expr.location}, {InternedString("Unsigned")}),
        std::vector<ast::ExprPtr>());
    error_ = TypeComparator(expected_unsigned_type_expr, ast_, &context_, &substitutor_, &z3_stuff_).Compare(*type_expr.parameters[1]);
    return !error_.has_value();
  }

  if (type_expr.parameters.size() != 1) {
    error_ = Error(CreateError() << "Expected 1 parmeter in Set constructor, but got " << type_expr.parameters.size());
    return false;
  }
  if (!std::holds_alternative<ast::TypeExpression>(*type_expr.parameters[0])) {
      error_ =  Error(CreateError() << "Expected first parmeter of Set to be a TypeExpression, but got " << *type_expr.parameters[0]);
      return false;
  }
  auto elem_type_expr = std::get<ast::TypeExpression>(*type_expr.parameters[0]);
  if (elem_type_expr.identifier.name.GetString() == "Array" || elem_type_expr.identifier.name.GetString() == "Set") {
    if (!CheckTypeExpression(elem_type_expr)) {
      return false;
    }
  }
  return true;
}

z3::sort TypeChecker::GetZ3SortForCollcections(const ast::TypeExpression &type_expr) {
  if (type_expr.identifier.name.GetString() == "Int" || type_expr.identifier.name.GetString() == "Unsigned") {      
    return z3_stuff_.context_.array_sort(z3_stuff_.context_.int_sort(), z3_stuff_.context_.int_sort());
  }
  if (type_expr.identifier.name.GetString() == "Bool") {
    return z3_stuff_.context_.array_sort(z3_stuff_.context_.int_sort(), z3_stuff_.context_.bool_sort());
  }
  if (type_expr.identifier.name.GetString() == "String") {
    return z3_stuff_.context_.array_sort(z3_stuff_.context_.int_sort(), z3_stuff_.context_.string_sort());
  }
  if (type_expr.identifier.name.GetString() == "Float") {
    return z3_stuff_.context_.array_sort(z3_stuff_.context_.int_sort(), z3_stuff_.context_.fpa_sort(11, 53));
  }
  if (type_expr.identifier.name.GetString() == "Array" || type_expr.identifier.name.GetString() == "Set")  {
    z3::sort elem_sort = GetZ3SortForCollcections(std::get<ast::TypeExpression>(*type_expr.parameters[0]));
    return z3_stuff_.context_.array_sort(z3_stuff_.context_.int_sort(), elem_sort);
  }
  return z3_stuff_.context_.array_sort(z3_stuff_.context_.int_sort(), z3_stuff_.sorts_.at(type_expr.identifier.name));
}

} // namespace dbuf::checker
