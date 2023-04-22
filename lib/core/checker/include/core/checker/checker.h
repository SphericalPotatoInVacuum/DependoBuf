#include "core/ast/ast.h"

namespace dbuf::checker {

class Checker {
public:
  explicit Checker(ast::AST &ast);

  void CheckAll();
  void CheckNameResolution();
  void CheckPositivity();
  void CheckTypeResolution();

private:
  ast::AST &ast_;
};

} // namespace dbuf::checker
