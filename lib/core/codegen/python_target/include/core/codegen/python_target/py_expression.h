#include "core/ast/ast.h"

#include <string>
#include <sstream>

namespace dbuf::gen {

class PyExpression {
public:
  PyExpression() = default;

  void set_constructor_type_map(const std::unordered_map<InternedString, InternedString> &constructor_to_type);

  std::string get_instances(const std::vector<std::shared_ptr<const ast::Expression>> &expressions);

  void operator()(const std::vector<std::shared_ptr<const ast::Expression>> &expressions);
  
  void operator()(const ast::Expression &expr);

  void operator()(const ast::BinaryExpression &bin_ex);

  void operator()(const ast::UnaryExpression &un_ex);

  void operator()(const ast::TypeExpression &typ_expr);

  void operator()(const ast::Value &val);

  void operator()(const ast::VarAccess &var_acc);

  void operator()(const ast::ScalarValue<bool> &scalar);

  void operator()(const ast::ScalarValue<std::string> &scalar);

  template<typename T>
  void operator()(const ast::ScalarValue<T> &scalar);

  void operator()(const ast::ConstructedValue &constructed);

private:
  std::stringstream buf_;

  static const std::unordered_map<char, std::string> kBinaryOperations;

  static const std::unordered_map<char, std::string> kUnaryOperations;

  static const std::unordered_map<bool, std::string> kBoolValues;

  std::shared_ptr<const std::unordered_map<InternedString, InternedString>> constructor_to_type_ = nullptr;
};

}