#include "core/checker/checker.h"

#include "core/ast/ast.h"
#include "core/interning/interned_string.h"

#include <algorithm>
#include <deque>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>
#include "core/checker/common.h"
#include "core/checker/positivity_checker.h"

#include <cstdlib>
#include <iostream>
#include <variant>

namespace dbuf::checker {

struct NameResolutionChecker {
  template <typename T>
  bool operator()(const T &) const {
    return false;
  }

  bool operator()(const ast::AST &ast) {
    AddScope();
    AddGlobalNames(ast);

    constructors_fields_ = GetConstructorFields(ast);

    if (!(*this)(ast.messages)) {
      return false;
    }

    return (*this)(ast.enums);
  }

  bool operator()(const std::unordered_map<InternedString, ast::Enum> &enums) {
    return !std::ranges::any_of(enums.begin(), enums.end(), [this](auto &ast_enum) {
      return !(*this)(ast_enum.second);
    });
  }

  bool operator()(const ast::Enum &ast_enum) {
    AddScope();
    if (!IsInScope(ast_enum.name)) {
      return false;
    }
    if (!(*this)(ast_enum.type_dependencies)) {
      return false;
    }
    if (!(*this)(ast_enum.pattern_mapping)) {
      return false;
    }
    DeleteScope();

    return true;
  }

  bool operator()(const std::vector<ast::Enum::Rule> &rules) {
    return !std::ranges::any_of(rules.begin(), rules.end(), [this](auto &rule) {
      return !(*this)(rule);
    });
  }

  bool operator()(const ast::Enum::Rule &rule) {
    AddScope();

    isShadowing_ = true;

    if (!(*this)(rule.inputs)) {
      return false;
    }

    isShadowing_ = false;

    if (!(*this)(rule.outputs)) {
      return false;
    }

    DeleteScope();

    return true;
  }

  bool operator()(std::vector<ast::Constructor> &constructors) {
    return !std::ranges::any_of(
        constructors.begin(),
        constructors.end(),
        [this](auto &constructor) { return !(*this)(constructor); });
  }

  bool operator()(const ast::Constructor &constructor) {
    return (!IsInScope(constructor.name)) && ((*this)(constructor.fields));
  }

  bool operator()(const ast::Star &) {
    return true;
  }

  bool operator()(const std::unordered_map<InternedString, ast::Message> &messages) {
    return !std::ranges::any_of(messages.begin(), messages.end(), [this](auto &message) {
      return !(*this)(message.second);
    });
  }

  bool operator()(const ast::Message &ast_message) {
    AddScope();
    if (!IsInScope(ast_message.name)) {
      return false;
    }
    if (!(*this)(ast_message.type_dependencies)) {
      return false;
    }
    if (!(*this)(ast_message.fields)) {
      return false;
    }
    DeleteScope();

    return true;
  }

  bool operator()(const std::vector<ast::TypedVariable> &dependencies) {
    return !std::ranges::any_of(dependencies.begin(), dependencies.end(), [this](auto &dependency) {
      return !(*this)(dependency);
    });
  }

  bool operator()(const ast::TypedVariable &typed_variable) {
    if (!(*this)(typed_variable.type_expression)) {
      return false;
    }
    return AddName(typed_variable.name);
  }

  bool operator()(const ast::TypeExpression &type_expression) {
    if (!IsInScope(type_expression.name)) {
      return false;
    }

    return (*this)(type_expression.parameters);
  }

  bool operator()(const std::vector<std::unique_ptr<ast::Expression>> &expressions) {
    for (auto &expression : expressions) {
      if (std::visit(*this, *expression)) {
        return false;
      }
    }
    return true;
  }

  bool operator()(const ast::ConstructedValue &value) {
    if (!IsInScope(value.constructor_identifier)) {
      return false;
    }
    AddScope();
    if (!(*this)(value.constructor_identifier, value.fields)) {
      return false;
    }
    DeleteScope();

    return true;
  }

  bool operator()(
      InternedString constructor_identifier,
      const std::vector<std::pair<InternedString, std::unique_ptr<ast::Expression>>> &fields) {
    for (auto &field : fields) {
      if (!constructors_fields_[constructor_identifier].contains(field.first)) {
        return false;
      }

      if (!(*this)(field)) {
        return false;
      }
    }
    return true;
  }

  bool operator()(const std::pair<InternedString, std::unique_ptr<ast::Expression>> &field) {
    if (!std::visit(*this, *field.second)) {
      return false;
    }
    AddName(field.first);
  }
  template <typename T>
  bool operator()(const ast::ScalarValue<T> &value) {
    return true;
  }

  bool operator()(const ast::BinaryExpression &expr) const {
    return std::visit(*this, *(expr.left)) && std::visit(*this, *expr.right);
  }
  bool operator()(const ast::UnaryExpression &expr) const {
    return std::visit(*this, *(expr.expression));
  }

  bool operator()(const ast::VarAccess &var_access) {
    return IsInScope(var_access.var_identifier);
  }

private:
  bool isShadowing_ = false;

  std::deque<std::unordered_set<InternedString>> scopes_;
  std::unordered_map<InternedString, std::unordered_set<InternedString>> constructors_fields_;
  std::unordered_map<InternedString, std::unordered_set<InternedString>> messages_fields_;

  bool IsInScope(InternedString name) {
    return std::ranges::any_of(scopes_.begin(), scopes_.end(), [&name](auto &scope) {
      return scope.contains(name);
    });
  }

  bool AddName(InternedString name) {
    if (scopes_.empty()) {
      throw std::logic_error("Can't add name to empty scopes.");
    }

    if ((!isShadowing_) && IsInScope(name)) {
      return false;
    }

    scopes_.back().insert(name);
    return true;
  }

  std::unordered_map<
      InternedString,
      std::unordered_set<InternedString>> static GetConstructorFields(const ast::AST &ast) {
    std::unordered_map<InternedString, std::unordered_set<InternedString>>
        constructor_to_fields_names;

    for (const auto &ast_enum : ast.enums) {
      for (const auto &pattern : ast_enum.second.pattern_mapping) {
        for (const auto &constructor : pattern.outputs) {
          for (const auto &field : constructor.fields) {
            constructor_to_fields_names[constructor.name].insert(field.name);
          }
        }
      }
    }

    return constructor_to_fields_names;
  }

  void AddGlobalNames(const ast::AST &ast) {
    AddName(InternedString("Int"));
    AddName(InternedString("String"));
    AddName(InternedString("Float"));

    for (const auto &ast_message : ast.messages) { AddName(ast_message.first); }

    for (const auto &ast_enum : ast.enums) {
      AddName(ast_enum.first);

      for (const auto &pattern : ast_enum.second.pattern_mapping) {
        for (const auto &constructor : pattern.outputs) { AddName(constructor.name); }
      }
    }
  }

  void AddScope() {
    scopes_.emplace_back(std::unordered_set<InternedString> {});
  }

  void DeleteScope() {
    if (scopes_.empty()) {
      throw std::logic_error("Can't delete scope from empty scopes.");
    }
    scopes_.pop_back();
  }
};

ErrorList Checker::CheckNameResolution(const ast::AST & /*ast*/) {
  // TODO (implement this)
  return {};
}

ErrorList Checker::CheckPositivity(const ast::AST &ast) {
  PositivityChecker::Result result = PositivityChecker()(ast);
  visit_order_                     = std::move(result.sorted);
  if (!result.errors.empty()) {
    return result.errors;
  }
  return {};
}

ErrorList Checker::CheckTypeResolution(const ast::AST & /*ast*/) {
  // TODO (implement this)
  return {};
}

int Checker::CheckAll(const ast::AST &ast) {
  CheckNameResolution(ast);
  ErrorList positivity_errors = CheckPositivity(ast);
  if (!positivity_errors.empty()) {
    for (const auto &error : positivity_errors) {
      std::cerr << error.message << std::endl;
    }
    return EXIT_FAILURE;
  }
  CheckTypeResolution(ast);

  return EXIT_SUCCESS;
}

} // namespace dbuf::checker
