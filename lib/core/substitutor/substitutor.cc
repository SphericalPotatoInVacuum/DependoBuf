#include "core/substitutor/substitutor.h"

namespace dbuf {

void Substitutor::AddSubstitution(InternedString name, ast::Expression &&expression) {
  substitute_[name] = std::visit(*this, std::move(expression));
}

ast::Expression Substitutor::operator()(const ast::BinaryExpression &expression) {
  ast::BinaryExpression res;

  res.type  = expression.type;
  res.left  = std::make_unique<ast::Expression>(std::visit(*this, *expression.left));
  res.right = std::make_unique<ast::Expression>(std::visit(*this, *expression.right));

  return res;
}
ast::Expression Substitutor::operator()(const ast::UnaryExpression &expression) {
  ast::UnaryExpression res;

  res.type       = expression.type;
  res.expression = std::make_unique<ast::Expression>(std::visit(*this, *expression.expression));

  return res;
}

ast::Expression Substitutor::operator()(const ast::Expression &, const ast::Expression &) {
  // TODO(alisa-vernigor): add error
  exit(1);
  return {};
}

ast::Expression Substitutor::operator()(const ast::Expression &, const ast::VarAccess &) {
  // TODO(alisa-vernigor): add error
  exit(1);
  return {};
}

ast::Expression
Substitutor::operator()(const ast::VarAccess &value, const ast::ConstructedValue &substitution) {
  if (value.field_identifiers.empty()) {
    return substitution;
  }

  size_t id = 0;
  for (id = 0; id < substitution.fields.size(); ++id) {
    if (substitution.fields[id].first == value.field_identifiers[0]) {
      break;
    }
  }

  const ast::Expression next = ast::VarAccess {
      .var_identifier    = value.field_identifiers[0],
      .field_identifiers = std::vector<InternedString>(
          value.field_identifiers.begin() + 1,
          value.field_identifiers.end())};

  return std::visit(*this, next, *substitution.fields[id].second);
}

ast::Expression
Substitutor::operator()(const ast::VarAccess &value, const ast::VarAccess &substitution) {
  std::vector<InternedString> fields = substitution.field_identifiers;
  fields.insert(fields.end(), value.field_identifiers.begin() + 1, value.field_identifiers.end());

  return ast::VarAccess {
      .var_identifier    = substitution.var_identifier,
      .field_identifiers = fields};
}

ast::Expression Substitutor::operator()(const ast::VarAccess &value) {
  if (!substitute_.contains(value.var_identifier)) {
    return value;
  }

  return std::visit(*this, ast::Expression(value), substitute_[value.var_identifier]);
}

ast::Expression Substitutor::operator()(const ast::ConstructedValue &value) {
  std::vector<std::pair<InternedString, std::shared_ptr<const ast::Expression>>> fields;
  fields.reserve(value.fields.size());
  for (const auto &field : value.fields) {
    fields.emplace_back(
        field.first,
        std::make_shared<const ast::Expression>(std::visit(*this, *field.second)));
  }

  ast::ConstructedValue res;
  res.constructor_identifier = value.constructor_identifier;
  res.fields                 = std::move(fields);

  return res;
}

ast::Expression Substitutor::operator()(const ast::Value &value) {
  return std::visit(*this, value);
}

ast::Expression Substitutor::operator()(const ast::TypeExpression &type_expression) {
  std::vector<std::shared_ptr<const ast::Expression>> parameters;
  parameters.reserve(type_expression.parameters.size());

  for (const auto &parameter : type_expression.parameters) {
    parameters.emplace_back(std::make_shared<const ast::Expression>(std::visit(*this, *parameter)));
  }

  ast::TypeExpression res;
  res.name       = type_expression.name;
  res.parameters = std::move(parameters);

  return res;
}

} // namespace dbuf
