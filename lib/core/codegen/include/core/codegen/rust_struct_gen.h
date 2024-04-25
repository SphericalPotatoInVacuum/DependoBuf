#include "core/ast/ast.h"

#include <ostream>

namespace dbuf::gen {
class RustStructGenerator {
public:
  explicit RustStructGenerator(std::ostream &output, const ast::AST &tree);

  void operator()(const ast::Message &ast_message);
  void operator()(const ast::Enum &ast_enum);

  void Generate();

private:
  std::ostream &output_;
  const ast::AST &tree_;

  bool IsPrimitiveType(const InternedString &type);
  std::string GetPrimitiveTypeInRust(const InternedString &type);
  std::string Tab(size_t n);
  void DeclareFields(const std::vector<ast::TypedVariable> &fields, size_t indent);

  const std::unordered_map<InternedString, std::string> kPrimitiveTypes_ {
      {InternedString("Int"), "i64"},
      {InternedString("Unsigned"), "u64"},
      {InternedString("Float"), "f64"},
      {InternedString("String"), "String"},
      {InternedString("Bool"), "bool"},
  };
};
} // namespace dbuf::gen