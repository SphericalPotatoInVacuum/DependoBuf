#include "core/ast/ast.h"
#include "core/interning/interned_string.h"

#include <ostream>

namespace dbuf::gen {
class RustCheckGenerator {
public:
  explicit RustCheckGenerator(std::ostream &output, const ast::AST &tree);

  void operator()(const ast::Message &ast_message);
  void operator()(const ast::Enum &ast_enum);

  void Generate();

private:
  std::ostream &output_;
  const ast::AST &tree_;
  const std::unordered_map<InternedString, std::string> kBuiltinToArgumentType_ = {
      {InternedString("String"), "&str"},
      {InternedString("Int"), "i64"},
      {InternedString("Unsigned"), "u64"},
      {InternedString("Float"), "f64"},
      {InternedString("Bool"), "bool"}};
};
} // namespace dbuf::gen