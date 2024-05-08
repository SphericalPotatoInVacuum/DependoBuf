#pragma once

#include "core/codegen/generation.h"

#include <unordered_set>
#include <vector>

namespace dbuf::gen {
class CppCodeGenerator : public ITargetCodeGenerator {
public:
  explicit CppCodeGenerator(const std::string &out_file)
      : ITargetCodeGenerator(out_file) {}

  void Generate(const ast::AST *tree) override;

  void
  operator()(const ast::Message &ast_message, std::vector<ast::TypedVariable> checker_input = {});

  void operator()(const ast::Enum &ast_enum, std::vector<ast::TypedVariable> &checker_input);

  void operator()(const ast::Enum &ast_enum);

  void operator()(const ast::TypedVariable &variable, bool as_dependency = false);

  void operator()(const ast::TypeExpression &expr, bool as_dependency = false);

  void operator()(const ast::BinaryExpression &expr);

  void operator()(const ast::UnaryExpression &expr);

  void operator()(const ast::Value &value);

  void operator()(const ast::ConstructedValue &value);

  void operator()(const ast::VarAccess &var_access);

  void operator()(const ast::Star &star);

  void PrintVariables(
      std::basic_ostream<char> &out,
      const std::vector<ast::TypedVariable> &variables,
      std::string delimeter,
      bool with_types,
      bool add_last_delimeter,
      bool as_dependency);

  bool CheckForTriggers(const std::unordered_set<InternedString> &trigger_names, const ast::Expression &expr);

  void CreateHiddenTypes(
      const std::vector<ast::TypedVariable> &fields,
      std::unordered_set<InternedString> trigger_names = {});

private:
  std::unordered_set<InternedString> created_hidden_types_;
  const ast::AST *tree_;
  int string_counter = 0;
};
} // namespace dbuf::gen