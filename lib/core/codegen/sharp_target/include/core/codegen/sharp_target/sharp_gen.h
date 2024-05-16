#pragma once

#include "core/codegen/generation.h"
#include "core/codegen/sharp_target/sharp_print.h"
#include "core/interning/interned_string.h"

#include <unordered_map>
#include <unordered_set>

namespace dbuf::gen {

struct VectorHash {
  std::size_t operator()(std::vector<std::pair<InternedString, InternedString>> &vec) const;

  std::size_t operator()(const std::vector<std::pair<InternedString, InternedString>> &vec) const;
};

class SharpCodeGenerator : public ITargetCodeGenerator {
public:
  explicit SharpCodeGenerator(const std::string &out_file);

  void Generate(const ast::AST *tree) override;

private:
  void operator()(const ast::Message &ast_message);

  void operator()(const ast::Message &ast_message, const std::vector<ast::TypedVariable> &checker_input);

  void operator()(const ast::Enum &ast_enum);

  void operator()(const ast::Enum &ast_enum, const std::vector<ast::TypedVariable> &checker_input);

  void operator()(const ast::TypedVariable &variable, bool as_dependency = false);

  void operator()(const ast::Expression &expr, bool as_dependency = false, bool need_access = true);

  void operator()(const ast::TypeExpression &expr, bool as_dependency = false, bool need_access = true);

  void operator()(const ast::BinaryExpression &expr);

  void operator()(const ast::UnaryExpression &expr);

  void operator()(const ast::ConstructedValue &value);

  void operator()(const ast::Value &value);

  void operator()(const ast::VarAccess &var_access);

  void operator()(const ast::Star &star);

  bool CheckForTriggers(const std::unordered_set<InternedString> &trigger_names, const ast::Expression &expr);

  void PrintTypedVariables(
      const std::vector<ast::TypedVariable> &variables,
      std::string &&delimeter,
      bool with_types,
      bool add_last_delimeter,
      bool as_dependency = false,
      bool need_access   = true);

  void PrintCheck(
      const std::unordered_map<InternedString, std::vector<std::shared_ptr<const ast::Expression>>> &checker_members,
      const std::vector<ast::TypedVariable> &checker_input);

  void PrintCheck(
    const std::vector<ast::TypedVariable> &checker_input,
    const std::vector<ast::Enum::Rule> &pattern_mapping,
    const std::vector<ast::TypedVariable> &original_dependencies,
    const std::string &added_name);

  static bool IsSimpleType(const InternedString &interned_string);

  const ast::AST *tree_;

  SharpPrinter printer_;

  std::unordered_set<InternedString> created_hidden_types_;
};

} // namespace dbuf::gen
