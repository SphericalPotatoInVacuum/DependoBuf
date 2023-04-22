#include "core/parser/parse_helper.h"

namespace dbuf::parser {
void ParseHelper::Parse() {
  parser_.set_debug_level(0);
  const int accept(0);
  if (parser_.parse() != accept) {
    throw "Parse failed!\n";
  }
}
} // namespace dbuf::parser
