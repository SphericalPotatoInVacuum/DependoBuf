#pragma once

#include "core/interning/interned_string.h"

#include <map>
#include <ostream>
#include <set>
#include <sstream>
#include <string>

namespace dbuf::checker {

using DependencyGraph = std::map<InternedString, std::set<InternedString>>;

struct Error { // NOLINT(bugprone-exception-escape)
  std::string message;
};

using ErrorList = std::vector<Error>;

class AddError {
public:
  explicit AddError(ErrorList *errors)
      : errors_(*errors) {}

  ~AddError() {
    errors_.emplace_back(Error {.message = ss_.str()});
  }

  template <typename T>
  AddError &operator<<(const T &obj) {
    ss_ << obj;
    return *this;
  }

private:
  ErrorList &errors_;
  std::ostringstream ss_;
};

} // namespace dbuf::checker
