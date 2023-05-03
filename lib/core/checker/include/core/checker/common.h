#pragma once

#include "core/interning/interned_string.h"

#include <map>
#include <set>
#include <string>

namespace dbuf::checker {

using DependencyGraph = std::map<InternedString, std::set<InternedString>>;

struct Error {
  std::string message;
};

using ErrorList = std::vector<Error>;

} // namespace dbuf::checker
