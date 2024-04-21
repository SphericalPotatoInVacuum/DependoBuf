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

  void operator()(const ast::Message& ast_message, std::vector<ast::TypedVariable> checker_input = {});

  void operator()(const ast::Enum &ast_enum);

  void operator()(const ast::TypedVariable &variable, bool as_dependency = false);

  void operator()(const ast::TypeExpression &expr, bool as_dependency = false);

  void operator()(const ast::BinaryExpression &expr);

  void operator()(const ast::UnaryExpression &expr);

  void operator()(const ast::Value &value);

  void operator()(const ast::ConstructedValue &value);

  void operator()(const ast::VarAccess &var_access);

private:
  std::unordered_set<InternedString> created_hidden_types_;
  const ast::AST* tree_;
};
} // namespace dbuf::gen