#include "core/codegen/py_gen.h"

namespace dbuf::gen {
void PyCodeGenerator::Generate(ast::AST *tree) {
  if (tree == nullptr) {
    std::cerr << "Python codegen did\'t get AST" << std::endl;
  }
  else {
    std::cout << "it's ok" << std::endl;
  }
}
} // namespace dbuf::gen
