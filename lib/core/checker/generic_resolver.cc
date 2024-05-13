#include "core/checker/generic_resolver.h"

#include "core/ast/ast.h"
#include "core/ast/expression.h"
#include "core/checker/common.h"
#include "core/interning/interned_string.h"
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
    } else if (std::holds_alternative<ast::Enum>(elem)) {
      const auto &en = std::get<ast::Enum>(elem);
      if (en.type_identifiers.empty()) {
        non_generic.types.emplace(name, elem);
      } else {
        generic.types.emplace(name, elem);
      }
    }
  }
  return std::make_pair(std::move(generic), std::move(non_generic));
}
} // namespace

#define MERGE_(a, b) a##b
#define LABEL_(a)    MERGE_(unique_name_, a)
#define UNIQUE_NAME  LABEL_(__LINE__)

#define ASSIGN_OR_RETURN_ERROR(type, lhs, rhs)                                                                         \
  std::variant<ErrorList, type> UNIQUE_NAME = (rhs);                                                                   \
  if (std::holds_alternative<ErrorList>(UNIQUE_NAME)) {                                                                \
    return std::get<ErrorList>(UNIQUE_NAME);                                                                           \
  }                                                                                                                    \
  type lhs = std::get<1>(UNIQUE_NAME);

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

std::variant<ErrorList, std::string> GenericsResolver::MapGenericWithParams(
    const std::vector<ast::Identifier> &identifiers,
    const std::vector<ast::TypeExpression> &generic_parameters) {
  std::string result_suffix = "<";
  for (size_t i = 0; i < identifiers.size(); ++i) {
    if (i >= generic_parameters.size()) {
      return ErrorList {Error {"GenericsResolver TypeExpression error: i >= type.parameters.size()"}};
    }
    const auto &param = generic_parameters[i];
    if (i != 0) {
      result_suffix += ",";
    }
    result_suffix += param.identifier.name.GetString();
    SCOPED_LOG(INFO) << identifiers[i].name << ": " << param.identifier.name;
    AddType(identifiers[i].name, param.identifier.name);
  }
  result_suffix += ">";
  current_generic_params_.emplace_back();
  for (const auto &iden : identifiers) {
    current_generic_params_.back().insert(iden.name);
  }
  return result_suffix;
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

  std::vector<ast::TypeExpression> generic_parameters;
  for (const auto &param_ptr : type.parameters) {
    const auto &param = *param_ptr;
    ASSIGN_OR_RETURN_ERROR(ast::Expression, param_result, Resolve(param));
    if (std::holds_alternative<ast::TypeExpression>(param_result)) {
      SCOPED_LOG(INFO) << "Type dependency";
      generic_parameters.emplace_back(std::get<ast::TypeExpression>(param_result));
    } else {
      SCOPED_LOG(INFO) << "Value dependency";
      result.parameters.emplace_back(std::make_shared<ast::Expression>(std::move(param_result)));
    }
  }
  if (auto it = name_to_generic_message_.find(type.identifier.name); it != name_to_generic_message_.end()) {
    SCOPED_LOG(INFO) << "GenericsResolver TypeExpression " << type.identifier.name << " is generic message\n";
    auto msg = it->second;
    ASSIGN_OR_RETURN_ERROR(std::string, suffix, MapGenericWithParams(msg.type_identifiers, generic_parameters));

    InternedString name(result.identifier.name.GetString() + suffix);
    result.identifier.name = name;
    if (resolved_generics_.contains(name)) {
      return result;
    }
    resolved_generics_.insert(name);
    ASSIGN_OR_RETURN_ERROR(ast::Message, resolved_msg, Resolve(msg));
    resolved_msg.identifier.name = name;
    result.identifier.name       = resolved_msg.identifier.name;
    result_ast_.types.emplace(name, std::move(resolved_msg));
    current_generic_params_.pop_back();
  } else {
    if (auto jt = name_to_generic_enum_.find(type.identifier.name); jt != name_to_generic_enum_.end()) {
      SCOPED_LOG(INFO) << "GenericsResolver TypeExpression " << type.identifier.name << " is generic enum\n";
      auto en = jt->second;
      ASSIGN_OR_RETURN_ERROR(std::string, suffix, MapGenericWithParams(en.type_identifiers, generic_parameters));
      InternedString name(result.identifier.name.GetString() + suffix);
      result.identifier.name = name;
      if (resolved_generics_.contains(name)) {
        return result;
      }
      resolved_generics_.insert(name);
      ASSIGN_OR_RETURN_ERROR(ast::Enum, resolved_enum, Resolve(en));
      resolved_enum.identifier.name = name;
      for (auto &rule : resolved_enum.pattern_mapping) {
        for (auto &constructor : rule.outputs) {
          constructor.identifier.name = InternedString(constructor.identifier.name.GetString() + suffix);
          result_ast_.constructor_to_type.emplace(constructor.identifier.name, name);
        }
      }
      result_ast_.types.emplace(name, std::move(resolved_enum));
      current_generic_params_.pop_back();

    } else {
      SCOPED_LOG(INFO) << "GenericsResolver TypeExpression " << type.identifier.name << " is not generic\n";
    }
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

std::variant<ErrorList, ast::Constructor> GenericsResolver::Resolve(const ast::Constructor &constr) {
  auto defer = MakeNewScope();
  SCOPED_LOG(INFO) << "GenericsResolver Constructor\n";
  ast::Constructor result;
  result.identifier = constr.identifier;
  for (const auto &field : constr.fields) {
    ASSIGN_OR_RETURN_ERROR(ast::TypedVariable, resolved_field, Resolve(field));
    result.fields.emplace_back(std::move(resolved_field));
  }
  return result;
}

std::variant<ErrorList, ast::Enum::Rule> GenericsResolver::Resolve(const ast::Enum::Rule &rule) {
  auto defer = MakeNewScope();
  SCOPED_LOG(INFO) << "GenericsResolver EnumRule\n";
  ast::Enum::Rule result;
  result.inputs = rule.inputs;
  for (const auto &out : rule.outputs) {
    ASSIGN_OR_RETURN_ERROR(ast::Constructor, res_out, Resolve(out));
    result.outputs.emplace_back(std::move(res_out));
  }
  return result;
}

std::variant<ErrorList, ast::Enum> GenericsResolver::Resolve(const ast::Enum &type) {
  auto defer = MakeNewScope();
  SCOPED_LOG(INFO) << "GenericsResolver Enum " << type.identifier.name;
  ast::Enum result;
  result.identifier = type.identifier;
  for (const auto &dep : type.type_dependencies) {
    ASSIGN_OR_RETURN_ERROR(ast::TypedVariable, resolved_dep, Resolve(dep));
    result.type_dependencies.emplace_back(std::move(resolved_dep));
  }
  for (const auto &elem : type.pattern_mapping) {
    ASSIGN_OR_RETURN_ERROR(ast::Enum::Rule, resolved_rule, Resolve(elem));
    result.pattern_mapping.emplace_back(std::move(resolved_rule));
  }
  return result;
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
      const auto &value                  = std::get<ast::Message>(generic.second);
      const auto &str                    = value.identifier;
      name_to_generic_message_[str.name] = value;
    } else if (std::holds_alternative<ast::Enum>(generic.second)) {
      const auto &en                  = std::get<ast::Enum>(generic.second);
      const auto &str                 = en.identifier;
      name_to_generic_enum_[str.name] = en;
      for (const auto &rule : en.pattern_mapping) {
        for (const auto &constructor : rule.outputs) {
          constructor_name_to_generic_enum_name_[constructor.identifier.name] = en.identifier.name;
        }
      }
    }
  }

  SCOPED_LOG(INFO) << "generic_.types.size() = " << generic_.types.size()
                   << ", non_generic_.types.size() = " << non_generic_.types.size();

  result_ast_.constructor_to_type = ast.constructor_to_type;
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
