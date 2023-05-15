#include "core/checker/type_comparator.h"

#include "core/ast/expression.h"
#include "core/checker/common.h"
#include "glog/logging.h"

#include <optional>

namespace dbuf::checker {

std::optional<Error> TypeComparator::Compare(const ast::Expression &expr) {
  auto result = std::visit(*this, expr);
  if (result) {
    DLOG(ERROR) << result->message;
  } else {
    DLOG(INFO) << "Expression " << expr << " is of expected type " << expected_;
  }
  return result;
}

std::optional<Error> TypeComparator::operator()(const ast::TypeExpression &) {
  DLOG(FATAL) << "TypeExpression should not be compared";
}
std::optional<Error> TypeComparator::operator()(const ast::BinaryExpression &expr) {
  DLOG(INFO) << "Checking binary expression " << expr;
  auto left_type_err = std::visit(*this, *expr.left);
  if (left_type_err) {
    return left_type_err;
  }
  auto right_type_err = std::visit(*this, *expr.right);
  if (right_type_err) {
    return right_type_err;
  }

  DLOG(INFO) << "Expression " << expr << " should be of type " << expected_;
  if (expr.type == ast::BinaryExpressionType::Plus) {
    if (expected_.identifier.name == InternedString("Int") || expected_.identifier.name == InternedString("Unsigned") ||
        expected_.identifier.name == InternedString("String") || expected_.identifier.name == InternedString("Float")) {
      return {};
    }
  }
  if (expr.type == ast::BinaryExpressionType::Star) {
    if (expected_.identifier.name == InternedString("Int") || expected_.identifier.name == InternedString("Unsigned") ||
        expected_.identifier.name == InternedString("Float")) {
      return {};
    }
  }
  if (expr.type == ast::BinaryExpressionType::Minus) {
    if (expected_.identifier.name == InternedString("Int") || expected_.identifier.name == InternedString("Float")) {
      return {};
    }
  }
  if (expr.type == ast::BinaryExpressionType::Slash) {
    if (expected_.identifier.name == InternedString("Float")) {
      return {};
    }
  }
  if (expr.type == ast::BinaryExpressionType::And || expr.type == ast::BinaryExpressionType::Or) {
    if (expected_.identifier.name == InternedString("Bool")) {
      return {};
    }
  }
  DLOG(ERROR) << "Invalid operator use in expression " << expr;
  return Error(
      CreateError() << "Operator \"" << static_cast<char>(expr.type) << "\""
                    << " is not supported by type " << expected_.identifier.name << "\" at " << expr.location);
  return {};
}
std::optional<Error> TypeComparator::operator()(const ast::UnaryExpression &expr) {
  DLOG(INFO) << "Checking unary expression " << expr;
  auto expr_err = std::visit(*this, *expr.expression);
  if (expr_err) {
    return expr_err;
  }
  if (expr.type == ast::UnaryExpressionType::Minus) {
    if (expected_.identifier.name == InternedString("Int") || expected_.identifier.name == InternedString("Float")) {
      return {};
    }
  }
  if (expr.type == ast::UnaryExpressionType::Bang) {
    if (expected_.identifier.name == InternedString("Bool")) {
      return {};
    }
  }
  DLOG(ERROR) << "Invalid operator use in expression " << expr;
  return Error(
      CreateError() << "Operator \"" << static_cast<char>(expr.type) << "\""
                    << " is not supported by type " << expected_.identifier.name << "\" at " << expr.location);
  return {};
}
std::optional<Error> TypeComparator::operator()(const ast::VarAccess &expr) {
  const Scope &outer_scope = *context_.back();
  DLOG(INFO) << "Checking var access: " << expr;
  // Case: does foo has type Foo?
  if (expr.field_identifiers.empty()) {
    // We can find foo in scope and just compare its type
    return CompareTypeExpressions(expected_, outer_scope.LookupName(expr.var_identifier.name), z3_stuff_);
  }
  // Else we have case: does foo.bar.buzz has type Buzz?

  // First lets find typename of foo. We have foo in scope, so we can get it from there
  InternedString message_name = outer_scope.LookupName(expr.var_identifier.name).identifier.name;
  // The field we are looking for. In case foo.bar.buzz we are looking for bar
  InternedString expected_field = expr.field_identifiers[0].name;

  const auto &type = ast_.types.at(message_name);
  if (std::holds_alternative<ast::Enum>(type)) {
    return Error(CreateError() << "Field access works only for messages, but \"" << message_name << "\" is enum");
    return {};
  }
  const auto &message = std::get<ast::Message>(type);

  bool found = false;

  // Let's notice the Type(foo.bar.buzz) == Type(bar.buzz) == Type (buzz)
  // We want to check type recursively, so I add new scope -- the scope of message Type(foo)
  Scope scope(&context_);
  for (const auto &dependencie : message.type_dependencies) {
    scope.AddName(dependencie.name, dependencie.type_expression);
  }
  for (const auto &field : message.fields) {
    scope.AddName(field.name, field.type_expression);
    if (field.name == expected_field) {
      found = true;
      break;
    }
  }

  // This case is not checked in name resolution checker, so I do it there
  if (!found) {
    return Error(CreateError() << "Field \"" << expected_field << "\" not found in message \"" << message_name << "\"");
  }

  // Recursion
  ast::VarAccess var_access;
  var_access.var_identifier    = {{parser::location()}, {expected_field}};
  var_access.field_identifiers = std::vector<ast::Identifier>();
  for (size_t id = 1; id < expr.field_identifiers.size(); ++id) {
    var_access.field_identifiers.push_back(expr.field_identifiers[id]);
  }

  (*this)(var_access);

  return {};
};

std::optional<Error> TypeComparator::operator()(const ast::Value &val) {
  return std::visit(*this, val);
}

std::optional<Error> TypeComparator::operator()(const ast::ConstructedValue &val) {
  const InternedString &enum_identifier = ast_.constructor_to_type.at(val.constructor_identifier.name);
  // TODO(bug): Constructed value can also be a message, not only enum

  if (enum_identifier != expected_.identifier.name) {
    return Error(
        CreateError() << "Got value of type \"" << enum_identifier << "\", but expected type is \""
                      << expected_.identifier.name << "\" at " << val.location);
    return {};
  }

  // TODO(alisa-vernigor): The pattern matching problem must be solved to check that the
  // constructor used creates the required type.
  return {};
}
} // namespace dbuf::checker
