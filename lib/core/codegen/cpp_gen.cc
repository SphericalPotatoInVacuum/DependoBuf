#include "core/codegen/generation.h"

namespace dbuf::gen {
void CppCodeGenerator::Generate(ast::AST *tree) {
  if (tree == nullptr) {
    std::cerr << "Cpp codegen did\'t get AST" << std::endl;
  }
}
} // namespace dbuf::gen