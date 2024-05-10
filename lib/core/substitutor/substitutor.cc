/*
This file is part of DependoBuf project.

Copyright (C) 2023 Alexander Bogdanov, Alice Vernigor

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/
#include "core/substitutor/substitutor.h"

#include "core/ast/ast.h"
#include "core/ast/expression.h"
#include "core/interning/interned_string.h"
#include "glog/logging.h"
#include "location.hh"

#include <cassert>
#include <cstddef>
#include <deque>
#include <memory>
#include <ranges>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

namespace dbuf {

Substitutor::Substitutor(const ast::AST &ast) : ast_(ast) {}

// Add a new (name -> expression) substitution to last scope
void Substitutor::AddSubstitution(InternedString name, const std::shared_ptr<const ast::Expression> &expression) {
  DCHECK(!substitute_.empty());
  DLOG(INFO) << "Adding substitution: " << name << " -> " << *expression;
  substitute_.back().insert_or_assign(name, std::move(expression));
}

// Add a new scope
void Substitutor::PushScope() {
  DLOG(INFO) << "Added a scope to substitutor";
  substitute_.emplace_back();
}

// Delete the last scope
void Substitutor::PopScope() {
  DCHECK(!substitute_.empty());
  DLOG(INFO) << "Popped a scope from substitutor";
  substitute_.pop_back();
}

// To substitute BinaryExpression, we need to substitute its left and right parts
ast::Expression Substitutor::operator()(const ast::BinaryExpression &expression) {
  return ast::BinaryExpression {
      {expression.location},
      expression.type,
      std::make_unique<ast::Expression>(std::visit(*this, *expression.left)),
      std::make_unique<ast::Expression>(std::visit(*this, *expression.right))};
}

// To substitute UnaryExpression, we need to substitute its expression part
ast::Expression Substitutor::operator()(const ast::UnaryExpression &expression) {
  return ast::UnaryExpression {
      {expression.location},
      expression.type,
      std::make_unique<ast::Expression>(std::visit(*this, *expression.expression))};
}

// To substitute TypeExpression we need to substitute all of its parameters
ast::Expression Substitutor::operator()(const ast::TypeExpression &type_expression) {
  std::vector<std::shared_ptr<const ast::Expression>> parameters;
  parameters.reserve(type_expression.parameters.size());

  for (const auto &parameter : type_expression.parameters) {
    parameters.emplace_back(std::make_shared<const ast::Expression>(std::visit(*this, *parameter)));
  }

  return ast::TypeExpression {{type_expression.location}, type_expression.identifier, std::move(parameters)};
}

// If VarAccess is in scope, we need to subsitute that VarAccess with corresponding Expression
ast::Expression Substitutor::operator()(const ast::VarAccess &value) {
  for (const auto &scope : std::ranges::reverse_view(substitute_)) {
    auto it = scope.find(value.var_identifier.name);
    if (it != scope.end()) {
      DLOG(INFO) << value;
      if (std::holds_alternative<ast::VarAccess>(*it->second) && std::get<ast::VarAccess>(*it->second).var_identifier.name == value.var_identifier.name) {
        return std::visit(*this, ast::Expression(value), *it->second);
      } 
      return std::visit(*this, ast::Expression(value), std::visit(*this, *it->second));
    }
  }
  return value;
}

// To substitute ArrayAccess we need to subtitute array_identifier and index
ast::Expression Substitutor::operator()(const ast::ArrayAccess &value) {
  ast::Expression array_identifier = std::visit(*this, *value.array_identifier);
  ast::Expression ind              = std::visit(*this, *value.ind);

  return ast::ArrayAccess {
      std::make_shared<const ast::Expression>(array_identifier),
      std::make_shared<const ast::Expression>(ind)};
}

ast::Expression Substitutor::operator()(const ast::Value &value) {
  return std::visit(*this, value);
}

// To substitute ConstructedValue we need to subtitute all fields of ConstructedValue
ast::Expression Substitutor::operator()(const ast::ConstructedValue &value) {
  std::vector<std::pair<ast::Identifier, ast::ExprPtr>> fields;
  fields.reserve(value.fields.size());
  for (const auto &field : value.fields) {
    fields.emplace_back(field.first, std::make_shared<const ast::Expression>(std::visit(*this, *field.second)));
  }

  return ast::ConstructedValue {{value.location}, value.constructor_identifier, std::move(fields)};
}

// To substitute CollectionValue we need to subtitute all elements of CollectionValue
ast::Expression Substitutor::operator()(const ast::CollectionValue &value) {
  std::vector<ast::ExprPtr> values;
  values.reserve(value.values.size());
  for (const auto &elem : value.values) {
    values.emplace_back(std::make_shared<const ast::Expression>(std::visit(*this, *elem)));
  }
  return ast::CollectionValue {{value.location}, std::move(values)};
}

ast::Expression Substitutor::operator()(const ast::FunctionValue &value) {
  PushScope();
  ast::Func func;
  std::deque<ast::ExprPtr> args;
  ast::FunctionValue current_value = value;
  while (true) {
    for (const auto& arg : std::ranges::reverse_view(current_value.args)) {
      args.push_front(arg);
    }
    if (ast_.functions.contains(current_value.function_identifier.name)) {
      func = ast_.functions.at(current_value.function_identifier.name);
      break;
    }
    for (const auto &scope : std::ranges::reverse_view(substitute_)) {
      auto it = scope.find(current_value.function_identifier.name);
      if (it != scope.end()) {
        current_value = std::get<ast::FunctionValue>(std::get<ast::Value>(*it->second));
        break;
      }
    }
  }
  for (size_t i = 0; i < args.size(); ++i) {
    AddSubstitution(func.type_dependencies[i].name, args[i]);
  }
  ast::Expression new_value = std::visit(*this, *func.return_value);
  PopScope();
  return new_value;
}

ast::Expression Substitutor::operator()(const ast::VarAccess &value, const ast::Value &substitution) {
  return std::visit(*this, ast::Expression(value), substitution);
}

// Case foo -> var1.var2 for foo.bar, expected result is var1.var2.bar
ast::Expression Substitutor::operator()(const ast::VarAccess &value, const ast::VarAccess &substitution) {
  std::vector<ast::Identifier> fields = substitution.field_identifiers;
  fields.insert(fields.end(), value.field_identifiers.begin(), value.field_identifiers.end());

  return ast::VarAccess {substitution.var_identifier, std::move(fields)};
}

// Case foo -> Foo {bar: some_value} and we need to substitute foo.bar
ast::Expression Substitutor::operator()(const ast::VarAccess &value, const ast::ConstructedValue &substitution) {
  if (value.field_identifiers.empty()) {
    return substitution;
  }

  size_t id = 0;
  for (id = 0; id < substitution.fields.size(); ++id) {
    if (substitution.fields[id].first.name == value.field_identifiers[0].name) {
      break;
    }
  }

  const ast::Expression next = ast::VarAccess {
      value.field_identifiers[0],
      std::vector<ast::Identifier>(value.field_identifiers.begin() + 1, value.field_identifiers.end())};

  return std::visit(*this, next, *substitution.fields[id].second);
}

ast::Expression Substitutor::operator()(const ast::VarAccess & /* value */, const ast::CollectionValue &substitution) {
  return substitution;
}

} // namespace dbuf
