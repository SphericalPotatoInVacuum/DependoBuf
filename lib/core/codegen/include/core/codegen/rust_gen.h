#include "core/ast/ast.h"
#include "core/codegen/generation.h"

namespace dbuf::gen {

class RustCodeGenerator : public ITargetCodeGenerator {
public:
  explicit RustCodeGenerator(const std::string &out_file, const bool is_testing = false)
      : ITargetCodeGenerator(out_file)
      , is_testing_(is_testing) {}

  void Generate(ast::AST *tree) override;

private:
  const bool is_testing_ = false;
};
} // namespace dbuf::gen
