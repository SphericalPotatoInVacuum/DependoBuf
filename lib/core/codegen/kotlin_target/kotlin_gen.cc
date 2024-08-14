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

  bool need_skip = false;
  for (auto &t : tree->visit_order) {
    if (need_skip) {
      printer_.NewLine();
      need_skip = false;
    }
    if (std::holds_alternative<ast::Message>(tree->types[t])) {
      auto msg = std::get<ast::Message>(tree->types[t]);
      printer_ << PrintableMessage(msg, tree);
      need_skip = true;
    }
  }
}

} // namespace dbuf::gen::kotlin
