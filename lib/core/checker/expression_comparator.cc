#include "core/checker/expression_comparator.h"

#include "core/ast/ast.h"
#include "core/checker/common.h"

#include <optional>

namespace dbuf::checker {

std::optional<Error> CompareExpressions(
    const ast::Expression &expected,
    const ast::Expression &got,
    Z3stuff &z3_stuff,
    const ast::AST &ast,
    std::deque<Scope *> &context) {
  DLOG(INFO) << "Comparing expressions " << expected << " and " << got;
  z3_stuff.solver_.push();

  ExpressionToZ3 converter {z3_stuff, ast, context};
  z3::expr expected_z3_expr = converter(expected);
  z3::expr got_z3_expr      = converter(got);

  DLOG(INFO) << "Z3 expressions: " << expected_z3_expr << " (expected) and " << got_z3_expr << " (got)";

  z3_stuff.solver_.add(expected_z3_expr != got_z3_expr);

  bool equal = z3_stuff.solver_.check() == z3::unsat;

  z3_stuff.solver_.pop();

  DLOG(INFO) << "Expressions " << expected << " and " << got << " are " << (equal ? "equal" : "not equal");

  if (!equal) {
    return Error(CreateError() << "Expressions " << expected << " and " << got << " are not equal");
  }
  return {};
}

} // namespace dbuf::checker
