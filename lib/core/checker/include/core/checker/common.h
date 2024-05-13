/*
This file is part of DependoBuf project.

Copyright (C) 2023 Alexander Bogdanov, Alice Vernigor

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/
#pragma once

#include "core/ast/ast.h"
#include "core/ast/expression.h"
#include "core/interning/interned_string.h"
#include "glog/logging.h"

#include <cassert>
#include <deque>
#include <functional>
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
    LOG(FATAL) << "Can't find name \"" << name.GetString() << "\"";
  }

private:
  std::unordered_map<InternedString, ast::TypeExpression> vars_;
  std::deque<Scope *> &ctx_;
};

class Defer {
public:
  explicit Defer(const std::function<void()> &other)
      : callback_(other) {}

  ~Defer() noexcept {
    callback_();
  }

private:
  std::function<void()> callback_;
};

// Find type of foo.bar.buzz
inline ast::TypeExpression
GetVarAccessType(const ast::VarAccess &var_access, const ast::AST &ast, std::deque<Scope *> *context_ptr) {
  // Get Type (Foo) of the head (foo)
  ast::TypeExpression head_type = context_ptr->back()->LookupName(var_access.var_identifier.name);

  // If no field identifiers
  if (var_access.field_identifiers.empty()) {
    return head_type;
  }

  // Find Foo message
  const auto &head_message = std::get<ast::Message>(ast.types.at(head_type.identifier.name));
  // Looking for head+1 (bar) id in Foo field
  size_t id;
  for (id = 0; id < head_message.fields.size(); ++id) {
    if (head_message.fields[id].name == var_access.field_identifiers[0].name) {
      break;
    }
  }

  // Add head+1 (bar) to scope of message Foo
  auto scope = Scope(context_ptr);
  scope.AddName(var_access.field_identifiers[0].name, head_message.fields[id].type_expression);

  // Building new VarAccess (bar.buzz)
  ast::VarAccess new_var_access = ast::VarAccess {
      .var_identifier = var_access.field_identifiers[0],
      .field_identifiers =
          std::vector<ast::Identifier>(var_access.field_identifiers.begin() + 1, var_access.field_identifiers.end())};

  // TypeOf(foo.bar.buzz) == TypeOf(bar.buzz from Foo), where TypeOf(foo) == Foo
  return GetVarAccessType(new_var_access, ast, context_ptr);
}

} // namespace dbuf::checker
