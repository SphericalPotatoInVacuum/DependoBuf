#pragma once

#include "core/ast/ast.h"
#include "core/codegen/generation.h"
#include "core/codegen/kotlin_target/kotlin_printer.h"

namespace dbuf::gen::kotlin {

class CodeGenerator : public ITargetCodeGenerator {
public:
  explicit CodeGenerator(const std::string &out_file);
  void Generate(ast::AST *tree);

private:
  Printer printer_;
};

} // namespace dbuf::gen::kotlin
