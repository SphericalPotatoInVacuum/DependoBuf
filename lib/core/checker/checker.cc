#include "core/checker/checker.h"

#include "core/ast/ast.h"

namespace dbuf::checker {

Checker::Checker(ast::AST &ast)
    : ast_(ast) {}

void Checker::CheckNameResolution() {
  // TODO (implement this)
}

void Checker::CheckPositivity() {
  // TODO (implement this)
}

void Checker::CheckTypeResolution() {
  // TODO (implement this)
}

void Checker::CheckAll() {
  CheckNameResolution();
  CheckPositivity();
  CheckTypeResolution();
}

} // namespace dbuf::checker
