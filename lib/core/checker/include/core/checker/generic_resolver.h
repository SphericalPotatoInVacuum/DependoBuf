#pragma once

#include "core/ast/ast.h"
#include "core/ast/expression.h"
#include "core/checker/common.h"
#include "core/interning/interned_string.h"

#include <unordered_map>
#include <variant>

namespace dbuf::checker {

class GenericsResolver {
public:
  std::variant<ErrorList, ast::AST> operator()(const ast::AST &ast);

  GenericsResolver();

private:
  std::variant<ErrorList, std::variant<ast::Message, ast::Enum>>
  Resolve(const std::variant<ast::Message, ast::Enum> &type);

  std::variant<ErrorList, ast::TypedVariable> Resolve(const ast::TypedVariable &type);

  std::variant<ErrorList, ast::TypeExpression> Resolve(const ast::TypeExpression &type);

  std::variant<ErrorList, ast::Expression> Resolve(const ast::Expression &type);

  std::variant<ErrorList, ast::Message> Resolve(const ast::Message &type);

  std::variant<ErrorList, ast::Enum> Resolve(const ast::Enum &type);

  Defer MakeNewScope();

  void AddType(const InternedString &param_name, const InternedString &type_name);

  std::optional<InternedString> GetType(const InternedString &param_name);

  static constexpr std::string_view kOneIndent = "  ";
  std::string current_indent_;

  std::vector<std::set<InternedString>> current_generic_params_;
  std::vector<std::unordered_map<InternedString, InternedString>> generic_param_to_type_name_;

  std::unordered_map<InternedString, ast::Message> name_to_generic_;
  ast::AST generic_;
  ast::AST non_generic_;
  ast::AST result_ast_;
};

} // namespace dbuf::checker
