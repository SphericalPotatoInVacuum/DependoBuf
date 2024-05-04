#pragma once

#include "core/ast/ast.h"
#include "core/codegen/generation.h"

namespace dbuf::gen {

class GoCodeGenerator : public dbuf::gen::ITargetCodeGenerator {
public:
  explicit GoCodeGenerator(const std::string &out_file)
      : ITargetCodeGenerator(out_file) {}

  void Generate(ast::AST *tree) override;

private:
  void GenerateObject(const ast::Message &);
  void GenerateObject(const ast::Enum &);
};

} // namespace dbuf::gen
