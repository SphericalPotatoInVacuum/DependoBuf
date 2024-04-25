#include "core/ast/ast.h"
#include "core/codegen/generation.h"

namespace dbuf::gen {

class RustCodeGenerator : public ITargetCodeGenerator {
public:
  explicit RustCodeGenerator(const std::string &out_file)
      : ITargetCodeGenerator(out_file) {}

  void Generate(ast::AST *tree) override;
};
} // namespace dbuf::gen
