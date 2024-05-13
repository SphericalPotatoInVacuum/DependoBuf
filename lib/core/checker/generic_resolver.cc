#include "core/checker/generic_resolver.h"

#include "core/ast/ast.h"
#include "core/ast/expression.h"
#include "core/checker/common.h"
#include "location.hh"

#include <iomanip>
#include <variant>

namespace dbuf::checker {

namespace {

std::pair<ast::AST, ast::AST> ClassifyGenerics(const ast::AST &ast) {
  ast::AST generic;
  ast::AST non_generic;
  for (const auto &[name, elem] : ast.types) {
    if (std::holds_alternative<ast::Message>(elem)) {
      const auto &msg = std::get<ast::Message>(elem);
      if (msg.type_identifiers.empty()) {
        non_generic.types.emplace(name, elem);
      } else {
        generic.types.emplace(name, elem);
      }
    }
  }
  return std::make_pair(std::move(generic), std::move(non_generic));
}
} // namespace

#define ASSIGN_OR_RETURN_ERROR(type, lhs, rhs)                                                                         \
  std::variant<ErrorList, type> result_or_error##__LINE__ = (rhs);                                                     \
  if (std::holds_alternative<ErrorList>(result_or_error##__LINE__)) {                                                  \
    return std::get<ErrorList>(result_or_error##__LINE__);                                                             \
  }                                                                                                                    \
  type lhs = std::get<1>(result_or_error##__LINE__);

#define SCOPED_LOG(severity) DLOG(severity) << (__LINE__ > 99 ? "" : "  ") << current_indent_

std::variant<ErrorList, ast::Expression> GenericsResolver::Resolve(const ast::Expression &type) {
  auto defer = MakeNewScope();
  SCOPED_LOG(INFO) << "GenericsResolver Expression " << type;
  if (std::holds_alternative<ast::TypeExpression>(type)) {
    ASSIGN_OR_RETURN_ERROR(ast::TypeExpression, result, Resolve(std::get<ast::TypeExpression>(type)));
    return result;
  }
  return type;
}

std::variant<ErrorList, ast::TypeExpression> GenericsResolver::Resolve(const ast::TypeExpression &type) {
  auto defer = MakeNewScope();
  SCOPED_LOG(INFO) << "GenericsResolver TypeExpression " << type.identifier.name;
  ast::TypeExpression result;
  result.identifier = type.identifier;
  if (auto it = current_generic_params_.back().find(result.identifier.name) != current_generic_params_.back().end()) {
    auto maybe_type = GetType(result.identifier.name);
    if (!maybe_type.has_value()) {
      return ErrorList {Error {"!maybe_type.has_value()"}};
    }
    result.identifier.name = maybe_type.value();
    return result;
  }
  // check if non
  std::vector<ast::TypeExpression> generic_parameters;
  for (const auto &param_ptr : type.parameters) {
    const auto &param = *param_ptr;
    ASSIGN_OR_RETURN_ERROR(ast::Expression, param_result, Resolve(param));
    if (std::holds_alternative<ast::TypeExpression>(param_result)) {
      generic_parameters.emplace_back(std::get<ast::TypeExpression>(param_result));
    } else {
      result.parameters.emplace_back(std::make_shared<ast::Expression>(std::move(param_result)));
    }
  }
  if (auto it = name_to_generic_.find(type.identifier.name); it != name_to_generic_.end()) {
    SCOPED_LOG(INFO) << "GenericsResolver TypeExpression " << type.identifier.name << " is generic\n";
    auto msg         = it->second;
    std::string name = msg.identifier.name.GetString() + "<";
    for (size_t i = 0; i < msg.type_identifiers.size(); ++i) {
      if (i >= generic_parameters.size()) {
        return ErrorList {Error {"GenericsResolver TypeExpression error: i >= type.parameters.size()"}};
      }
      const auto &param = generic_parameters[i];
      // if (!type_expr.parameters.empty()) {
      //   return ErrorList {Error {"GenericsResolver TypeExpression error: !type_expr.parameters.empty()"}};
      // }
      if (i != 0) {
        name += ",";
      }
      name += param.identifier.name.GetString();
      SCOPED_LOG(INFO) << msg.type_identifiers[i].name << ": " << param.identifier.name;
      AddType(msg.type_identifiers[i].name, param.identifier.name);
    }
    name += ">";
    result.identifier.name = InternedString(name);
    current_generic_params_.emplace_back();
    for (const auto &iden : msg.type_identifiers) {
      current_generic_params_.back().insert(iden.name);
    }
    ASSIGN_OR_RETURN_ERROR(ast::Message, resolved_msg, Resolve(msg));
    resolved_msg.identifier.name = result.identifier.name;
    result_ast_.types.emplace(result.identifier.name, std::move(resolved_msg));
    current_generic_params_.pop_back();
  } else {
    SCOPED_LOG(INFO) << "GenericsResolver TypeExpression " << type.identifier.name << " is not generic\n";
  }
  SCOPED_LOG(INFO) << "GenericsResolver TypeExpression result name is " << result.identifier.name << "\n";
  return result;
}

std::variant<ErrorList, ast::TypedVariable> GenericsResolver::Resolve(const ast::TypedVariable &type) {
  auto defer = MakeNewScope();
  SCOPED_LOG(INFO) << "GenericsResolver TypedVariable " << type.name;
  ast::TypedVariable result;
  result.name = type.name;
  ASSIGN_OR_RETURN_ERROR(ast::TypeExpression, tmp, Resolve(type.type_expression));
  result.type_expression = tmp;
  return result;
}

std::variant<ErrorList, ast::Message> GenericsResolver::Resolve(const ast::Message &type) {
  auto defer = MakeNewScope();
  SCOPED_LOG(INFO) << "GenericsResolver Message " << type.identifier.name;
  ast::Message result;
  result.identifier = type.identifier;
  if (!result.type_identifiers.empty()) {
    return ErrorList {Error {std::string("!result.type_identifiers.empty()")}};
  }
  for (const auto &dep : type.type_dependencies) {
    ASSIGN_OR_RETURN_ERROR(ast::TypedVariable, resolved_dep, Resolve(dep));
    result.type_dependencies.emplace_back(std::move(resolved_dep));
  }
  for (const auto &field : type.fields) {
    ASSIGN_OR_RETURN_ERROR(ast::TypedVariable, resolved_field, Resolve(field));
    result.fields.emplace_back(std::move(resolved_field));
  }
  return result;
}

std::variant<ErrorList, ast::Enum> GenericsResolver::Resolve(const ast::Enum &type) {
  auto defer = MakeNewScope();
  SCOPED_LOG(INFO) << "GenericsResolver Enum " << type.identifier.name;
  return type;
}

std::variant<ErrorList, std::variant<ast::Message, ast::Enum>>
GenericsResolver::Resolve(const std::variant<ast::Message, ast::Enum> &type) {
  auto defer = MakeNewScope();
  SCOPED_LOG(INFO) << "GenericsResolver Message/Enum\n";
  if (std::holds_alternative<ast::Message>(type)) {
    ASSIGN_OR_RETURN_ERROR(ast::Message, result, Resolve(std::get<ast::Message>(type)));
    return result;
  }
  ASSIGN_OR_RETURN_ERROR(ast::Enum, result, Resolve(std::get<ast::Enum>(type)));
  return result;
}

std::variant<ErrorList, ast::AST> GenericsResolver::operator()(const ast::AST &ast) {
  auto defer = MakeNewScope();
  SCOPED_LOG(INFO) << "GenericsResolver AST\n";
  std::tie(generic_, non_generic_) = ClassifyGenerics(ast);
  for (const auto &generic : generic_.types) {
    if (std::holds_alternative<ast::Message>(generic.second)) {
      const auto &value          = std::get<ast::Message>(generic.second);
      const auto &str            = value.identifier;
      name_to_generic_[str.name] = value;
    }
  }

  SCOPED_LOG(INFO) << "generic_.types.size() = " << generic_.types.size()
                   << ", non_generic_.types.size() = " << non_generic_.types.size();
  for (const auto &[name, elem] : non_generic_.types) {
    std::variant<ErrorList, std::variant<ast::Message, ast::Enum>> result = Resolve(elem);
    if (std::holds_alternative<ErrorList>(result)) {
      return std::get<ErrorList>(result);
    }
    result_ast_.types.emplace(name, std::get<std::variant<ast::Message, ast::Enum>>(result));
  }
  return result_ast_;
}

GenericsResolver::GenericsResolver() {
  generic_param_to_type_name_.resize(1);
  current_generic_params_.resize(1);
}

void GenericsResolver::AddType(const InternedString &param_name, const InternedString &type_name) {
  assert(!generic_param_to_type_name_.empty());
  generic_param_to_type_name_.back()[param_name] = type_name;
}

std::optional<InternedString> GenericsResolver::GetType(const InternedString &param_name) {
  assert(!generic_param_to_type_name_.empty());
  if (auto it = generic_param_to_type_name_.back().find(param_name); it != generic_param_to_type_name_.back().end()) {
    return it->second;
  }
  return std::nullopt;
}

Defer GenericsResolver::MakeNewScope() {
  current_indent_ += kOneIndent;
  auto new_scope = generic_param_to_type_name_.back(); // TODO(gmusya): improve
  generic_param_to_type_name_.emplace_back(std::move(new_scope));
  return Defer([this]() {
    assert(current_indent_.size() >= kOneIndent.size());
    current_indent_.resize(current_indent_.size() - kOneIndent.size());
    assert(generic_param_to_type_name_.size() >= 2);
    generic_param_to_type_name_.pop_back();
  });
}

} // namespace dbuf::checker
