#include "core/codegen/python_target/py_expression.h"
#include "core/codegen/python_target/utils.h"
#include "core/ast/ast.h"

#include <string>

namespace dbuf::gen {

  PyExpression::PyExpression(const std::unordered_map<InternedString, InternedString> &constructor_to_type) {
    constructor_to_type_ = std::make_shared<const std::unordered_map<InternedString, InternedString>>(constructor_to_type);
  }

  std::string PyExpression::operator()(std::vector<std::shared_ptr<const ast::Expression>> &expressions) {
    std::vector<std::string> instances;
    for (const auto expr: expressions) {
      std::string inst = (*this)(*expr);
    }
  }

  std::string PyExpression::operator()(const ast::Expression &expr) {
    std::visit(*this, expr);
  }

  std::string PyExpression::operator()(const ast::BinaryExpression &bin_ex) {
    std::string left = (*this)(*bin_ex.left);
    std::string op = kBinaryOperations.at(static_cast<char>(bin_ex.type));
    std::string right = (*this)(*bin_ex.right);

    res_.clear();
    res_ << left << " " << op << " " << right;
    return res_.str();
  }

  std::string PyExpression::operator()(const ast::UnaryExpression &un_ex) {
    std::string op = kUnaryOperations.at(static_cast<char>(un_ex.type));
    std::string expression = (*this)(*un_ex.expression);

    res_.clear();
    res_ << op << expression;
    return res_.str();
  }

  std::string PyExpression::operator()(const ast::Value &val) {
    std::visit(*this, val);
  }

  std::string PyExpression::operator()(const ast::VarAccess &var_acc) {
    res_.clear();
    res_ << var_acc;
    return res_.str();
  }

  std::string PyExpression::operator()(const ast::ScalarValue<bool> &scalar) {
    return kBoolValues.at(scalar.value);
  }

  template<typename T>
  std::string PyExpression::operator()(const ast::ScalarValue<T> &scalar) {
    return to_string(scalar.value);
  }

  std::string PyExpression::operator()(const ast::ConstructedValue &constructed) {
    const InternedString &interned_constructor_name = constructed.constructor_identifier.name;
    const std::string &struct_name = constructor_to_type_->at(interned_constructor_name).GetString();
    const std::string &constructor_name = interned_constructor_name.GetString();

    res_.clear();
    res_ << struct_name << ".__" << constructor_name << "(";

    std::string sep = ", ";
    for (int i = 0; i < constructed.fields.size(); ++i) {
      if (i != 0) {
        res_ << sep;
      }
      std::string field_val = (*this)(*constructed.fields[i].second);
      res_ << field_val;
    }
    res_ << ")";
    return res_.str();
  }

  const std::unordered_map<char, std::string> PyExpression::kBinaryOperations = {
    {'+', "+"},
    {'-', "-"},
    {'*', "*"},
    {'/', "/"},
    {'&', "and"},
    {'|', "or"},
  };

  const std::unordered_map<char, std::string> PyExpression::kUnaryOperations = {
    {'-', "-"},
    {'!', "not "},
  };

  const std::unordered_map<bool, std::string> PyExpression::kBoolValues = {
    {true, "True"},
    {false, "False"},
  };
}