#pragma once

#include "core/ast/ast.h"
#include "core/ast/expression.h"
#include "core/checker/common.h"
#include "core/interning/interned_string.h"
#include "core/substitutor/substitutor.h"

#include <deque>
#include <string>
#include <unordered_map>
#include <vector>

namespace dbuf::checker {

class TypeExpressionChecker {
  // template <typename T>
  //  void operator()(const ast::TypeExpression &, const T &) {}

  explicit TypeExpressionChecker(ast::AST &ast, std::vector<InternedString> &sorted_graph);

  // The enterpoint
  void CheckTypes() {
    for (const auto &node : sorted_graph_) {
      // Checking message
      if (ast_.messages.contains(node)) {
        // Scope of the message checked
        PushScope();
        // Iterator to message from ast map
        auto it = ast_.messages.find(node);
        // First step is to check if dependencies' types are correctly constructed
        for (const auto &dependency : it->second.type_dependencies) {
          (*this)(dependency.type_expression);

          // After we checked the depdency we can add it to scope to be seen by other dependencies
          AddName(dependency.name, dependency.type_expression);
        }

        // Same with fields
        for (const auto &field : it->second.fields) {
          (*this)(field.type_expression);

          // After we checked the depdency we can add it to scope to be seen by other dependencies
          AddName(field.name, field.type_expression);
        }

        // Clear used scope
        PopScope();
      } else {
      }
    }
  }

  void operator()(const ast::TypeExpression &type_expression) {
    // Typename should be the name of some message/enum
    if (!IsTypeName(type_expression.identifier.name)) {
      errors_.emplace_back(Error {
          .message = "Unknown typename: \"" + type_expression.identifier.name.GetString() + "\""});
    }

    // If it is a message
    auto it_message = ast_.messages.find(type_expression.identifier.name);
    if (it_message != ast_.messages.end()) {
      // Number of parameters should be equal to number of dependencies of message/enum
      if (it_message->second.type_dependencies.size() != type_expression.parameters.size()) {
        errors_.emplace_back(Error {
            .message = "Expected " + std::to_string(it_message->second.type_dependencies.size()) +
                       " parameters for typename \"" + type_expression.identifier.name.GetString() +
                       "\", but got " + std::to_string(type_expression.parameters.size())});
        return;
      }

      // Next step is to check parameters
      (*this)(it_message->second.type_dependencies, type_expression.parameters);
    }

    auto it_enum = ast_.enums.find(type_expression.identifier.name);
    if (it_enum != ast_.enums.end()) {
      (*this)(it_enum->second.type_dependencies, type_expression);
    }
  }

  //
  void operator()(
      const std::vector<ast::TypedVariable> &type_dependencies,
      const std::vector<std::shared_ptr<const ast::Expression>> &type_parameters) {
    substitutor_.ClearSubstitutionMap();
    for (size_t id = 0; id < type_dependencies.size(); ++id) {
      // Update type using already known substitutions
      ast::Expression substituted_type = substitutor_(type_dependencies[id].type_expression);

      // Compare types
      (*this)(substituted_type, *type_parameters[id]);

      // Now we can update substitutions
      substitutor_.AddSubstitution(type_dependencies[id].name, type_parameters[id]);
    }
  }

  void operator()(const ast::TypeExpression &expected_type, const ast::VarAccess &expression);

  void operator()(const ast::Expression &expected, const ast::Expression &got) {
    // TODO(SphericalPotatoInVacuum): compare expressions
  }

  void operator()(const ast::TypeExpression &expected_type, const ast::TypeExpression &expression);

  void operator()(const ast::TypeExpression &expected_type, const ast::ConstructedValue &value);

  template <typename T>
  void operator()(const ast::TypeExpression &expected_type, const ast::ScalarValue<T> &value) {
    if (expected_type.identifier.name != GetTypename(value)) {
      errors_.emplace_back(Error {
          .message = "Got value of type \"" + GetTypename(value).GetString() +
                     "\", but expected type is \"" + expected_type.identifier.name.GetString() +
                     "\""});
      return;
    }
  }

private:
  void GetConstructorToEnum();

  [[nodiscard]] bool IsTypeName(InternedString name) const;

  void AddName(InternedString name, ast::TypeExpression type);

  bool IsInScope(InternedString name);

  void PushScope();

  void PopScope();

  template <typename T>
  InternedString GetTypename(const ast::ScalarValue<T> &value) {}

  template <>
  InternedString GetTypename<bool>(const ast::ScalarValue<bool> &) {
    return InternedString("Bool");
  }
  template <>
  InternedString GetTypename<int64_t>(const ast::ScalarValue<int64_t> &) {
    return InternedString("Int");
  }
  template <>
  InternedString GetTypename<uint64_t>(const ast::ScalarValue<uint64_t> &) {
    return InternedString("Unsigned");
  }
  template <>
  InternedString GetTypename<double>(const ast::ScalarValue<double> &) {
    return InternedString("Float");
  }
  template <>
  InternedString GetTypename<std::string>(const ast::ScalarValue<std::string> &) {
    return InternedString("String");
  }

  Substitutor substitutor_;
  std::vector<InternedString> sorted_graph_;
  std::deque<std::unordered_map<InternedString, ast::TypeExpression>> context_;
  ast::AST &ast_;
  ErrorList errors_;
  std::unordered_map<InternedString, InternedString> constructor_to_enum_;
};

} // namespace dbuf::checker
