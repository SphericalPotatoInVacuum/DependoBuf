#include "core/ast/ast.h"
#include "core/ast/expression.h"
#include "core/codegen/generation.h"
#include "core/codegen/python_target/py_expression.h"
#include "core/codegen/python_target/py_print.h"

namespace dbuf::gen {

class PyCodeGenerator : public ITargetCodeGenerator {
public:
  explicit PyCodeGenerator(const std::string &out_file);

  void Generate(ast::AST *tree) override;

  void operator()(const ast::Message &ast_message);

  void operator()(const ast::Enum &ast_enum);

private:
  static std::tuple<std::string, std::string> get_name_and_type(const ast::TypedVariable &var);

  void prepare_names_types_deps_for_deps(
      const std::vector<ast::TypedVariable> &typed_vars,
      std::vector<std::string> &names,
      std::vector<std::string> &types,
      std::vector<std::string> &deps,
      std::vector<bool> &need_ignores);

  void prepare_names_types_deps_for_fields(
      const std::vector<ast::TypedVariable> &typed_vars,
      std::vector<std::string> &names,
      std::vector<std::string> &types,
      std::vector<std::string> &deps,
      std::vector<bool> &need_ignores);

  PyPrinter printer_;

  PyExpression py_exression_;
};

} // namespace dbuf::gen