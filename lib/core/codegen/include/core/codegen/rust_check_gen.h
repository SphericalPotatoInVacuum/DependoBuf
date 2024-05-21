#include "core/ast/ast.h"
#include "core/ast/expression.h"
#include "core/interning/interned_string.h"

#include <ostream>
#include <vector>

namespace dbuf::gen {
class RustCheckGenerator {
public:
  explicit RustCheckGenerator(std::ostream &output, const ast::AST &tree, bool is_testing);

  void operator()(const ast::Message &ast_message);
  void operator()(const ast::Enum &ast_enum);

  void Generate();

private:
  std::ostream &output_;
  const ast::AST &tree_;
  const bool is_testing_ = false;

  void DeclareCheckStart(const InternedString &name, const std::vector<ast::TypedVariable> &deps);
  void DeclareCheckEnd();

  void DeclareMatchCheckStart(const InternedString &name, size_t index, const std::vector<ast::TypedVariable> &deps);
  void DeclareMatchCheckEnd();

  void CheckFields(const std::vector<ast::TypedVariable> &fields, const std::string_view &indent);

  void PrintExpression(const ast::Expression &expr);
  void PrintConstructedValue(const ast::ConstructedValue &expr);

  bool IsCopyableType(const InternedString &type);
  bool IsPrimitiveType(const InternedString &type);

  const std::unordered_map<InternedString, std::string> kBuiltinToArgumentType_ = {
      {InternedString("Int"), "i64"},
      {InternedString("Unsigned"), "u64"},
      {InternedString("Float"), "f64"},
      {InternedString("Bool"), "bool"}};
};
} // namespace dbuf::gen
