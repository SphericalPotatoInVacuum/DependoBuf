#include "core/ast/ast.h"
#include "core/codegen/generation.h"

namespace dbuf::gen {

class CppCodeGenerator : public ITargetCodeGenerator {
public:
  explicit CppCodeGenerator(const std::string &out_file)
      : ITargetCodeGenerator(out_file) {}

  void operator()(const ast::Message &ast_message);

  void operator()(const ast::Enum &ast_enum);

  void Generate(ast::AST *tree) override;
};
} // namespace dbuf::gen