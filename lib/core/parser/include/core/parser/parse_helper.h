#include "core/parser/lexer.h"
#include "dbuf.tab.hpp"

namespace dbuf::parser {

class ParseHelper {
public:
  ParseHelper(std::istream &in, std::ostream &out, ast::AST *ast)
      : lexer_(in, out)
      , parser_(&lexer_, ast) {};

  void Parse();

private:
  Lexer lexer_;
  Parser parser_;
};

} // namespace dbuf::parser
