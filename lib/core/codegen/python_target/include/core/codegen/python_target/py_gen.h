#include "core/ast/ast.h"
#include "core/ast/expression.h"
#include "core/codegen/generation.h"
#include "core/codegen/python_target/py_print.h"
#include "core/codegen/python_target/py_expression.h"

namespace dbuf::gen {

class PyCodeGenerator : public ITargetCodeGenerator {
public:
  explicit PyCodeGenerator(const std::string &out_file);

  void Generate(ast::AST *tree) override;

  void operator()(const ast::Message &ast_message);

  void operator()(const ast::Enum &ast_enum);

private:
  static std::tuple<std::string, std::string> get_name_and_type(const ast::TypedVariable &var);

  void prepare_deps(
      const std::vector<ast::TypedVariable>& type_dependencies,
      std::vector<std::string> &dep_names,
      std::vector<std::string> &dep_types,
      std::vector<std::string> &dep_deps);

  PyPrinter printer_;

  PyExpression py_exression_;
};

} // namespace dbuf::gen