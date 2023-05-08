#pragma once

#include "core/ast/ast.h"
#include "core/checker/common.h"
#include "core/interning/interned_string.h"

namespace dbuf::checker {

class Checker {
public:
  using ErrorList = std::vector<Error>;

  static ErrorList CheckNameResolution(const ast::AST &ast);
  ErrorList CheckPositivity(const ast::AST &ast);
  static ErrorList CheckTypeResolution(const ast::AST &ast, const std::vector<InternedString> &visit_order);

  int CheckAll(const ast::AST &ast);

private:
  std::vector<InternedString> visit_order_;
};

} // namespace dbuf::checker
