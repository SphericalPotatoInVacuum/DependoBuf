/*
This file is part of DependoBuf project.

Copyright (C) 2023 Alexander Bogdanov, Alice Vernigor

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/
#include "core/parser/parse_helper.h"

namespace dbuf::parser {
void ParseHelper::Parse() {
  parser_.set_debug_level(0);
  const int accept(0);
  if (parser_.parse() != accept || parser_.GetErrorCnt() > 0) {
    throw "Parse failed!
";
  }
}
} // namespace dbuf::parser
