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

  explicit operator Error() const;

private:
  std::ostringstream ss_;
};

class Scope {
public:
  explicit Scope(std::deque<Scope *> *ctx_ptr);

  ~Scope();

  // Delete the copy constructor and copy assignment operator to prevent copying
  Scope(const Scope &)            = delete;
  Scope &operator=(const Scope &) = delete;

  void AddName(InternedString name, ast::TypeExpression type);

  [[nodiscard]] const ast::TypeExpression &LookupName(InternedString name) const;

private:
  std::unordered_map<InternedString, ast::TypeExpression> vars_;
  std::deque<Scope *> &ctx_;
};

// Find type of foo.bar.buzz
ast::TypeExpression
GetVarAccessType(const ast::VarAccess &var_access, const ast::AST &ast, std::deque<Scope *> *context_ptr);

} // namespace dbuf::checker
