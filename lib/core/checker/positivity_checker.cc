#include "core/checker/positivity_checker.h"

#include <sstream>

namespace dbuf::checker {

void PositivityChecker::operator()(const ast::Value & /* value */) {}

void PositivityChecker::operator()(const ast::VarAccess & /* var_access */) {}

void PositivityChecker::operator()(const ast::UnaryExpression &expr) {
  std::visit(*this, *expr.expression);
}

void PositivityChecker::operator()(const ast::BinaryExpression &expr) {
  std::visit(*this, *expr.left);
  std::visit(*this, *expr.right);
}

void PositivityChecker::operator()(const ast::TypeExpression &type_expression) {
  dependency_graph_[current_type_].insert(type_expression.name);
  for (const auto &parameter : type_expression.parameters) {
    std::visit(*this, *parameter);
  }
}

PositivityChecker::Result PositivityChecker::operator()(const ast::AST &ast) {
  for (const auto &[_, message] : ast.messages) {
    current_type_ = message.name;
    for (const auto &dep : message.type_dependencies) {
      (*this)(dep.type_expression);
    }
  }

  return TopSortGraph();
}

PositivityChecker::Result PositivityChecker::TopSortGraph() const {
  std::vector<InternedString> sorted;
  std::unordered_map<InternedString, NodeState> node_states;

  std::stringstream message_stream;
  Result result;

  for (const auto &[name, _] : dependency_graph_) {
    if (!node_states.contains(name)) {
      std::vector<InternedString> cycle = Visit(name, sorted, node_states);

      if (!cycle.empty()) {
        message_stream << "Found dependency cycle: ";

        bool first_node = true;
        for (const auto &node : cycle) {
          if (!first_node) {
            message_stream << " -> ";
          }
          message_stream << node.GetString();
          first_node = false;
        }

        result.errors.push_back({message_stream.str()});
        return result;
      }
    }
  }

  result.sorted = std::move(sorted);

  return result;
}

std::vector<InternedString> PositivityChecker::Visit( // NOLINT(misc-no-recursion)
    const InternedString &name,
    std::vector<InternedString> &sorted,
    std::unordered_map<InternedString, NodeState> &node_states) const {
  if (!dependency_graph_.contains(name)) {
    return {};
  }

  node_states[name] = NodeState::Visiting;

  for (const auto &dep : dependency_graph_.at(name)) {
    if (node_states.contains(dep)) {                    // We're either visiting or have visited
      if (node_states.at(dep) == NodeState::Visiting) { // We're visiting => cycle
        return {dep, name};
      }
    } else { // We haven't visited this node yet
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
