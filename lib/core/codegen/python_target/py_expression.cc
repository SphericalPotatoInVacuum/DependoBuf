#include "core/codegen/python_target/py_expression.h"
#include "core/codegen/python_target/utils.h"
#include "core/ast/ast.h"

#include <string>

namespace dbuf::gen {

  void PyExpression::set_constructor_type_map(const std::unordered_map<InternedString, InternedString> &constructor_to_type) {
    constructor_to_type_ = std::make_shared<const std::unordered_map<InternedString, InternedString>>(constructor_to_type);
  }

  std::string PyExpression::get_instances(const std::vector<std::shared_ptr<const ast::Expression>> &expressions) {
    (*this)(expressions);
    std::string res = buf_.str();
    buf_.str("");
    return res;
  }

  std::string PyExpression::get_instances(const std::vector<std::variant<ast::Value, ast::Star>> &expressions) {
    (*this)(expressions);
    std::string res = buf_.str();
    buf_.str("");
    return res;
  }

  void PyExpression::operator()(const std::vector<std::shared_ptr<const ast::Expression>> &expressions) {
    buf_ << "(";
    std::string sep = ", ";
    for (int i = 0; i < expressions.size(); ++i) {
      (*this)(*expressions[i]);
      buf_ << sep;
    }
    buf_ << ")";
  }

  void PyExpression::operator()(const std::vector<std::variant<ast::Value, ast::Star>> &expressions) {
    buf_ << "(";
    std::string sep = ", ";
    for (int i = 0; i < expressions.size(); ++i) {
      std::visit(*this, expressions[i]);
      buf_ << sep;
    }
    buf_ << ")";
  }

  void PyExpression::operator()(const ast::Expression &expr) {
    std::visit(*this, expr);
  }

  void PyExpression::operator()(const ast::BinaryExpression &bin_ex) {
    if (need_brackets_) {
      buf_ << "(";
    }
    bool cur_need_brackets = need_brackets_;
    need_brackets_ = true;

    (*this)(*bin_ex.left);

    std::string op = kBinaryOperations.at(static_cast<char>(bin_ex.type));
    buf_ << " " << op << " ";

    (*this)(*bin_ex.right);

    need_brackets_ = cur_need_brackets;
    if (need_brackets_) {
      buf_ << ")";
    }
  }

  void PyExpression::operator()(const ast::UnaryExpression &un_ex) {
    if (need_brackets_) {
      buf_ << "(";
    }
    bool cur_need_brackets = need_brackets_;
    need_brackets_ = true;

    std::string op = kUnaryOperations.at(static_cast<char>(un_ex.type));
    buf_ << op;

    (*this)(*un_ex.expression);

    need_brackets_ = cur_need_brackets;
    if (need_brackets_) {
      buf_ << ")";
    }
  }

  void PyExpression::operator()(const ast::Value &val) {
    std::visit(*this, val);
  }

  void PyExpression::operator()(const ast::VarAccess &var_acc) {
    buf_ << var_acc;
  }

  void PyExpression::operator()(const ast::ScalarValue<int64_t> &scalar) {
    if (scalar.value >= 0) {
      buf_ << scalar.value;
      return;
    }

    buf_ << "(" << scalar.value << ")";
  }

  void PyExpression::operator()(const ast::ScalarValue<bool> &scalar) {
    buf_ << kBoolValues.at(scalar.value);
  }

  void PyExpression::operator()(const ast::ScalarValue<std::string> &scalar) {
    buf_ << "'" << scalar.value << "'";
  }

  template<typename T>
  void PyExpression::operator()(const ast::ScalarValue<T> &scalar) {
    buf_ << scalar.value;
  }

  void PyExpression::operator()(const ast::ConstructedValue &constructed) {
    const InternedString &interned_constructor_name = constructed.constructor_identifier.name;
    const std::string &struct_name = constructor_to_type_->at(interned_constructor_name).GetString();
    const std::string &constructor_name = interned_constructor_name.GetString();

    buf_ << struct_name << "._" << struct_name << "__" << constructor_name << "(";

    std::string sep = ", ";
    for (int i = 0; i < constructed.fields.size(); ++i) {
      if (i != 0) {
        buf_ << sep;
      }
      (*this)(*constructed.fields[i].second);
    }
    buf_ << ")";
  }

  void PyExpression::operator()(const ast::Star &star) {
    buf_ << "None";
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