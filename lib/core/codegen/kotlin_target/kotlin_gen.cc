#include "core/codegen/kotlin_target/kotlin_gen.h"

#include "core/ast/ast.h"
#include "core/codegen/kotlin_target/kotlin_objects.h"
#include "core/codegen/kotlin_target/kotlin_printer.h"

#include <format>

namespace dbuf::gen::kotlin {

CodeGenerator::CodeGenerator(const std::string &out_file)
    : ITargetCodeGenerator(out_file)
    , printer_(output_) {}

void CodeGenerator::Generate(ast::AST *tree) {
  std::cerr << "types:\n";
  for (auto &[a, b] : tree->types) {
    if (std::holds_alternative<ast::Message>(b)) {
      std::cerr << "  Message [" << a << "]\n";
    }
    if (std::holds_alternative<ast::Enum>(b)) {
      std::cerr << "  Enum [" << a << "]\n";
    }
  }
  std::cerr << "constructors to type:\n";
  for (auto &[a, b] : tree->constructor_to_type) {
    std::cerr << "  " << a << " to " << b << "\n";
  }
  std::cerr << "visit order\n";
  for (auto &a : tree->visit_order) {
    std::cerr << "  " << a << "\n";
  }

  for (const auto &t : tree->visit_order) {
    if (std::holds_alternative<ast::Message>(tree->types[t])) {
      const auto &message = std::get<ast::Message>(tree->types[t]);
      printer_ << PrintableMessage(message, tree);
    }
    if (std::holds_alternative<ast::Enum>(tree->types[t])) {
      const auto &ast_enum = std::get<ast::Enum>(tree->types[t]);
      printer_ << PrintableEnum(ast_enum, tree);
    }
  }
}

} // namespace dbuf::gen::kotlin
