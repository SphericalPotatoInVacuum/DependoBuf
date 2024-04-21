#include "core/ast/ast.h"
#include "core/ast/expression.h"
#include "core/codegen/generation.h"
#include "core/codegen/py_print.h"

namespace dbuf::gen {

class PyCodeGenerator : public ITargetCodeGenerator {
public:
  explicit PyCodeGenerator(const std::string &out_file);

  void Generate(ast::AST *tree) override;

private:
  void operator()(const ast::Message &ast_message);

  void operator()(const ast::Enum &ast_enum);

  void operator()(const ast::TypedVariable &typed_var);

  void operator()(const ast::TypeExpression &typed_expr);

  void operator()(const ast::Expression &expr);

  void operator()(const ast::VarAccess &var_access);

  void operator()(const ast::BinaryExpression &bin_expr);

  void operator()(const ast::UnaryExpression &un_expr);

  void operator()(const ast::Value &value);

  void operator()(const ast::ConstructedValue &constructed_val);

  template <typename T>
  void operator()(const ast::ScalarValue<T> &scalar_val);

  void operator()(const ast::Star &value);

  PyPrinter printer_;
};

} // namespace dbuf::gen