/*
This file is part of DependoBuf project.

Copyright (C) 2023 Alexander Bogdanov, Alice Vernigor

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/
#include "core/checker/common.h"

#include "glog/logging.h"

namespace dbuf::checker {

CreateError::operator Error() const {
  return Error {ss_.str()};
}

Scope::Scope(std::deque<Scope *> *ctx_ptr)
    : ctx_(*ctx_ptr) {
  ctx_.push_back(this);
  DLOG(INFO) << "Added a scope to type checker";
}

Scope::~Scope() {
  DCHECK(this == ctx_.back());
  ctx_.pop_back();
  DLOG(INFO) << "Popped a scope from type checker";
}

void Scope::AddName(InternedString name, ast::TypeExpression type) {
  vars_.insert_or_assign(name, std::move(type));
  DLOG(INFO) << "Added name \"" << name << "\" with type \"" << vars_.at(name) << "\" to context";
}

[[nodiscard]] const ast::TypeExpression &Scope::LookupName(InternedString name) const {
  for (const auto &scope : std::ranges::reverse_view(ctx_)) {
    auto it = scope->vars_.find(name);
    if (it != scope->vars_.end()) {
      return it->second;
    }
  }
  LOG(FATAL) << "Can't find name \"" << name.GetString() << "\"";
}

ast::TypeExpression
GetVarAccessType(const ast::VarAccess &var_access, const ast::AST &ast, std::deque<Scope *> *context_ptr) {
  ast::TypeExpression head_type = context_ptr->back()->LookupName(var_access.var_identifier.name);
  if (var_access.field_identifiers.empty()) {
    return head_type;
  }
  const auto &head_message = std::get<ast::Message>(ast.types.at(head_type.identifier.name));
  size_t id;
  for (id = 0; id < head_message.fields.size(); ++id) {
    if (head_message.fields[id].name == var_access.field_identifiers[0].name) {
      break;
    }
  }
  auto scope = Scope(context_ptr);
  scope.AddName(var_access.field_identifiers[0].name, head_message.fields[id].type_expression);
  return GetVarAccessType(
      ast::VarAccess {
          var_access.field_identifiers[0],
          std::vector<ast::Identifier>(var_access.field_identifiers.begin() + 1, var_access.field_identifiers.end())},
      ast,
      context_ptr);
}

} // namespace dbuf::checker
