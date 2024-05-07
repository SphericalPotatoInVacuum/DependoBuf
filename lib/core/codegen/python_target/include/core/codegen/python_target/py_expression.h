#include "core/ast/ast.h"

#include <string>
#include <sstream>

namespace dbuf::gen {

class PyExpression {
public:
  explicit PyExpression(const std::unordered_map<InternedString, InternedString> &constructor_to_type);

  std::string operator()(const ast::Expression &expr);

private:
  std::string operator()(const ast::BinaryExpression &bin_ex);

  std::string operator()(const ast::UnaryExpression &un_ex);

  std::string operator()(const ast::Value &val);

  std::string operator()(const ast::VarAccess &var_acc);

  std::string operator()(const ast::ScalarValue<bool> &scalar);

  std::string operator()(const ast::ConstructedValue &constructed);

  std::stringstream res_;

  static const std::unordered_map<char, std::string> kBinaryOperations;

  static const std::unordered_map<char, std::string> kUnaryOperations;

  const std::unordered_map<InternedString, InternedString> &constructor_to_type_;
};

}