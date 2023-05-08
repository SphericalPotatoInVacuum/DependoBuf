#include "core/parser/parse_helper.h"

namespace dbuf::parser {
void ParseHelper::Parse() {
  parser_.set_debug_level(0);
  const int accept(0);
  if (parser_.parse() != accept || parser_.GetErrorCnt() > 0) {
    throw "Parse failed!\n";
  }
}
} // namespace dbuf::parser
