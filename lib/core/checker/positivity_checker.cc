/*
This file is part of DependoBuf project.

Copyright (C) 2023 Alexander Bogdanov, Alice Vernigor

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/
#include "core/checker/positivity_checker.h"

#include "glog/logging.h"

#include <map>
#include <ranges>
#include <sstream>

namespace dbuf::checker {

PositivityChecker::Result PositivityChecker::operator()(const ast::AST &ast) {
  DLOG(INFO) << "Running positivity checker";
  for (const auto &[_, type] : ast.types) {
    std::visit(*this, type);
  }
  return TopSortGraph();
}

void PositivityChecker::operator()(const ast::Message &ast_message) {
  DLOG(INFO) << "Checking message: " << ast_message.identifier.name;
  current_type_ = ast_message.identifier.name;
  add_self_     = true;
  dependency_graph_.emplace(current_type_, std::set<InternedString> {});
  for (const auto &dep : ast_message.type_dependencies) {
    (*this)(dep.type_expression);
  }
  add_self_ = false;
  for (const auto &field : ast_message.fields) {
    (*this)(field.type_expression);
  }
}

void PositivityChecker::operator()(const ast::Enum &ast_enum) {
  DLOG(INFO) << "Checking enum: " << ast_enum.identifier.name;
  current_type_ = ast_enum.identifier.name;
  add_self_     = true;
  dependency_graph_.emplace(current_type_, std::set<InternedString> {});
  for (const auto &dep : ast_enum.type_dependencies) {
    (*this)(dep.type_expression);
  }
  for (const auto &rule : ast_enum.pattern_mapping) {
    add_self_ = true;
    for (const auto &input : rule.inputs) {
      std::visit(*this, input);
    }
    add_self_ = false;
    for (const auto &output : rule.outputs) {
      for (const auto &field : output.fields) {
        (*this)(field.type_expression);
      }
    }
  }
}

void PositivityChecker::operator()(const ast::TypeExpression &type_expression) {
  if (add_self_ || (type_expression.identifier.name != current_type_)) {
    dependency_graph_.at(current_type_).insert(type_expression.identifier.name);
    DLOG(INFO) << "Adding dependency: " << current_type_ << " -> " << type_expression.identifier.name;
  }
  for (const auto &parameter : type_expression.parameters) {
    std::visit(*this, *parameter);
  }
}

void PositivityChecker::operator()(const ast::Value & /* value */) {}

void PositivityChecker::operator()(const ast::VarAccess & /* var_access */) {}

void PositivityChecker::operator()(const ast::ArrayAccess & /* array_access */) {}

void PositivityChecker::operator()(const ast::Star & /* star */) {}

void PositivityChecker::operator()(const ast::BinaryExpression &expr) {
  std::visit(*this, *expr.left);
  std::visit(*this, *expr.right);
}

void PositivityChecker::operator()(const ast::UnaryExpression &expr) {
  std::visit(*this, *expr.expression);
}

PositivityChecker::Result PositivityChecker::TopSortGraph() const {
  std::vector<InternedString> sorted;
  std::map<InternedString, NodeState> node_states;
  std::stringstream message_stream;
  Result result;
  for (const auto &[type_name, _] : dependency_graph_) {
    if (!node_states.contains(type_name)) {
      std::vector<InternedString> cycle = Visit(type_name, sorted, node_states);
      if (!cycle.empty()) {
        message_stream << "Found dependency cycle: ";
        bool first_node = true;
        for (const auto &node : std::ranges::reverse_view(cycle)) {
          if (!first_node) {
            message_stream << " -> ";
          }
          message_stream << node.GetString();
          first_node = false;
        }
        result.errors.push_back(Error {message_stream.str()});
        return result;
      }
    }
  }
  result.sorted = std::move(sorted);
  return result;
}

std::vector<InternedString> PositivityChecker::Visit(
    const InternedString &name,
    std::vector<InternedString> &sorted,
    std::map<InternedString, NodeState> &node_states) const {
  if (!dependency_graph_.contains(name)) {
    return {};
  }
  node_states[name] = NodeState::Visiting;
  for (const auto &dep : dependency_graph_.at(name)) {
    if (node_states.contains(dep)) {
      if (node_states.at(dep) == NodeState::Visiting) {
        return {dep, name};
      }
    } else {
      std::vector<InternedString> cycle = Visit(dep, sorted, node_states);
      if (!cycle.empty()) {
        if (cycle.back() != cycle.front()) {
          cycle.push_back(name);
        }
        return cycle;
      }
    }
  }
  node_states[name] = NodeState::Visited;
  sorted.push_back(name);
  return {};
}

} // namespace dbuf::checker
