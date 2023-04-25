#pragma once

#include "core/interning/interned_string.h"

#include <string>
#include <unordered_map>
#include <unordered_set>

namespace dbuf::checker {

using DependencyGraph = std::unordered_map<InternedString, std::unordered_set<InternedString>>;

struct Error {
  std::string message;
};

using ErrorList = std::vector<Error>;

} // namespace dbuf::checker
