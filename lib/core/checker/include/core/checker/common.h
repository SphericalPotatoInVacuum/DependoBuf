#pragma once

#include "core/ast/ast.h"
#include "core/interning/interned_string.h"
#include "glog/logging.h"

#include <cassert>
#include <deque>
#include <map>
#include <ostream>
#include <ranges>
#include <set>
#include <sstream>
#include <string>

namespace dbuf::checker {

using DependencyGraph = std::map<InternedString, std::set<InternedString>>;

struct Error { // NOLINT(bugprone-exception-escape)
  std::string message;
};

using ErrorList = std::vector<Error>;

class CreateError {
public:
  CreateError() = default;

  template <typename T>
  CreateError &operator<<(const T &obj) {
    ss_ << obj;
    return *this;
  }

  explicit operator Error() const {
    return Error {.message = ss_.str()};
  }

private:
  std::ostringstream ss_;
};

class Scope {
public:
  explicit Scope(std::deque<Scope *> *ctx_ptr)
      : ctx_(*ctx_ptr) {
    ctx_.push_back(this);
    DLOG(INFO) << "Added a scope to type checker";
  }
  ~Scope() {
    DCHECK(this == ctx_.back());
    ctx_.pop_back();
    DLOG(INFO) << "Popped a scope from type checker";
  }

  // Delete the copy constructor and copy assignment operator to prevent copying
  Scope(const Scope &)            = delete;
  Scope &operator=(const Scope &) = delete;

  void AddName(InternedString name, ast::TypeExpression type) {
    vars_.insert_or_assign(name, std::move(type));
    DLOG(INFO) << "Added name \"" << name << "\" with type \"" << vars_.at(name) << "\" to context";
  }
  [[nodiscard]] const ast::TypeExpression &LookupName(InternedString name) const {
    for (const auto &scope : std::ranges::reverse_view(ctx_)) {
      auto it = scope->vars_.find(name);
      if (it != scope->vars_.end()) {
        return it->second;
      }
    }
    DLOG(FATAL) << "Can't find name \"" << name.GetString() << "\"";
  }

private:
  std::unordered_map<InternedString, ast::TypeExpression> vars_;
  std::deque<Scope *> &ctx_;
};

} // namespace dbuf::checker
