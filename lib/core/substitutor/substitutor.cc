/*
This file is part of DependoBuf project.

Copyright (C) 2023 Alexander Bogdanov, Alice Vernigor

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/
#include "core/substitutor/substitutor.h"

#include "core/ast/expression.h"
#include "glog/logging.h"
#include "location.hh"

#include <cassert>
#include <memory>
#include <ranges>
#include <stdexcept>

namespace dbuf {

// Add a new (name -> expression) substitution to last scope
void SubstitutorScope::AddSubstitution(InternedString name, const std::shared_ptr<const ast::Expression> &expression) {
  DCHECK(!substitute_.empty());
  DLOG(INFO) << "Adding substitution: " << name << " -> " << *expression;
  auto expr = std::make_shared<ast::Expression>(std::visit(*this, *expression));
  DLOG(INFO) << "Added substitution: " << name << " -> " << *expr;
  substitute_.insert_or_assign(name, std::move(expr));
}

// If we want to substitute a binary expression, we need to substitute its left and right parts
ast::Expression SubstitutorScope::operator()(const ast::BinaryExpression &expression) {
  return ast::BinaryExpression {
      {expression.location},
      expression.type,
      std::make_unique<ast::Expression>(std::visit(*this, *expression.left)),
      std::make_unique<ast::Expression>(std::visit(*this, *expression.right))};
}

ast::Expression SubstitutorScope::operator()(const ast::VarAccess &value, const ast::Value &substitution) {
  return std::visit(*this, ast::Expression(value), substitution);
}

// If we want to substitute an unary expression, we just need to substitute its expression part
ast::Expression SubstitutorScope::operator()(const ast::UnaryExpression &expression) {
  return ast::UnaryExpression {
      {expression.location},
      expression.type,
      std::make_unique<ast::Expression>(std::visit(*this, *expression.expression))};
}

ast::Expression SubstitutorScope::operator()(const ast::VarAccess &value, const ast::ConstructedValue &substitution) {
  // If field identifiers are empty, we have the case: n -> Succ {prev: 5},
  // so we just return Succ {prev: 5} as a result
  if (value.field_identifiers.empty()) {
    return substitution;
  }

  // Else we have case foo -> Foo {bar: some_value} and we need to substitute foo.bar
  // First we need to find bar field in the substitution
  size_t id = 0;
  for (id = 0; id < substitution.fields.size(); ++id) {
    if (substitution.fields[id].first.name == value.field_identifiers[0].name) {
      break;
    }
  }

  // Let's notice that foo.bar.buzz.far with Foo {bar: {buzz: varible}} should return the same
  // expression as bar.buzz with Bar {buzz: variable}. The expected result in both cases is
  // variable.bar. That means, that we can go deeper by one field each time
  const ast::Expression next = ast::VarAccess {
      value.field_identifiers[0],
      std::vector<ast::Identifier>(value.field_identifiers.begin() + 1, value.field_identifiers.end())};

  return std::visit(*this, next, *substitution.fields[id].second);
}

// Case foo -> var1.var2 for foo.bar, expected result if var1.var2.bar
ast::Expression SubstitutorScope::operator()(const ast::VarAccess &value, const ast::VarAccess &substitution) {
  // Push substitution fields (var1.var2) first and when all fields except first (bar) of value
  std::vector<ast::Identifier> fields = substitution.field_identifiers;
  fields.insert(fields.end(), value.field_identifiers.begin(), value.field_identifiers.end());

  // So we return var1.var2.bar
  return ast::VarAccess {substitution.var_identifier, fields};
}

ast::Expression SubstitutorScope::operator()(const ast::VarAccess &value) {
  for (const auto &scope : std::ranges::reverse_view(ctx_)) {
    auto it = scope->substitute_.find(value.var_identifier.name);
    if (it != scope->substitute_.end()) {
      return std::visit(*this, ast::Expression(value), *it->second);
    }
  }

  return value;
}

// To substitute constucted value we need to subtitute all fields of constructed value
ast::Expression SubstitutorScope::operator()(const ast::ConstructedValue &value) {
  std::vector<std::pair<ast::Identifier, std::shared_ptr<const ast::Expression>>> fields;
  fields.reserve(value.fields.size());
  for (const auto &field : value.fields) {
    fields.emplace_back(field.first, std::make_shared<const ast::Expression>(std::visit(*this, *field.second)));
  }

  ast::ConstructedValue res;
  res.constructor_identifier.name = value.constructor_identifier.name;
  res.fields                      = std::move(fields);

  return res;
}

ast::Expression SubstitutorScope::operator()(const ast::Value &value) {
  return std::visit(*this, value);
}

// To substitute type_expression we need to substitute all of its parameters
ast::Expression SubstitutorScope::operator()(const ast::TypeExpression &type_expression) {
  std::vector<std::shared_ptr<const ast::Expression>> parameters;
  parameters.reserve(type_expression.parameters.size());

  for (const auto &parameter : type_expression.parameters) {
    parameters.emplace_back(std::make_shared<const ast::Expression>(std::visit(*this, *parameter)));
  }

  ast::TypeExpression res {
      {parser::location()},
      {parser::location(), type_expression.identifier.name},
      std::move(parameters)};

  return res;
}

} // namespace dbuf
