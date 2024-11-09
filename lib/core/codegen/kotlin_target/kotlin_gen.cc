#include "core/codegen/kotlin_target/kotlin_gen.h"

#include "core/ast/ast.h"
#include "core/codegen/kotlin_target/kotlin_objects.h"
#include "core/codegen/kotlin_target/kotlin_printer.h"

#include <format>

namespace dbuf::gen::kotlin {

CodeGenerator::CodeGenerator(const std::string &out_file)
    : ITargetCodeGenerator(out_file)
    , printer_(output_) {}

void CodeGenerator::Generate(const ast::AST *tree) {
  for (const auto &t : tree->visit_order) {
    if (std::holds_alternative<ast::Message>(tree->types.at(t))) {
      const auto &message = std::get<ast::Message>(tree->types.at(t));
      printer_ << PrintableMessage(message, tree);
    } else if (std::holds_alternative<ast::Enum>(tree->types.at(t))) {
      const auto &ast_enum = std::get<ast::Enum>(tree->types.at(t));
      printer_ << PrintableEnum(ast_enum, tree);
    } else {
      throw std::string("kotlin code generation: unknow variant of AST.types.second");
    }
  }
}

} // namespace dbuf::gen::kotlin
