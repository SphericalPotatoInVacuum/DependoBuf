#include "core/ast/ast.h"

#include <sstream>
#include <string>

namespace dbuf::gen {

class PyExpression {
public:
  PyExpression() = default;

  void set_constructor_type_map(const std::unordered_map<InternedString, InternedString> &constructor_to_type);

  std::string get_instances(const std::vector<std::shared_ptr<const ast::Expression>> &expressions);

  std::string get_instances(const std::vector<std::variant<ast::Value, ast::Star>> &expressions);

  void operator()(const std::vector<std::shared_ptr<const ast::Expression>> &expressions);

  void operator()(const std::vector<std::variant<ast::Value, ast::Star>> &expressions);

  void operator()(const ast::Expression &expr);

  void operator()(const ast::BinaryExpression &bin_ex);

  void operator()(const ast::UnaryExpression &un_ex);

  void operator()(const ast::Value &val);

  void operator()(const ast::VarAccess &var_acc);

  void operator()(const ast::ScalarValue<int64_t> &scalar);

  void operator()(const ast::ScalarValue<bool> &scalar);

  void operator()(const ast::ScalarValue<std::string> &scalar);

  template <typename T>
  void operator()(const ast::ScalarValue<T> &scalar);

  void operator()(const ast::ConstructedValue &constructed);

  void operator()(const ast::Star &star);

private:
  std::stringstream buf_;

  bool need_brackets_ = false;

  static const std::unordered_map<char, std::string> kBinaryOperations;

  static const std::unordered_map<char, std::string> kUnaryOperations;

  static const std::unordered_map<bool, std::string> kBoolValues;

  std::shared_ptr<const std::unordered_map<InternedString, InternedString>> constructor_to_type_ = nullptr;
};

} // namespace dbuf::gen