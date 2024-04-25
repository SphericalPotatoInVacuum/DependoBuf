#include "core/codegen/rust_struct_gen.h"

#include "core/interning/interned_string.h"

#include <variant>

namespace dbuf::gen {

RustStructGenerator::RustStructGenerator(std::ostream &output, const ast::AST &tree)
    : output_(output)
    , tree_(tree) {}

bool RustStructGenerator::IsPrimitiveType(const InternedString &type) {
  return kPrimitiveTypes_.contains(type);
}

std::string RustStructGenerator::GetPrimitiveTypeInRust(const InternedString &type) {
  return kPrimitiveTypes_.at(type);
}

inline std::string Tab(size_t n) {
  return std::string(n, ' ');
}

void RustStructGenerator::DeclareFields(const std::vector<ast::TypedVariable> &fields, const size_t indent) {
  for (const auto &field : fields) {
    output_ << Tab(indent) << field.name << ": ";
    const auto &dbuf_name = field.type_expression.identifier.name;
    if (IsPrimitiveType(dbuf_name)) {
      output_ << GetPrimitiveTypeInRust(dbuf_name);
    } else {
      output_ << "Box< " << dbuf_name << ">";
    }
    output_ << ",\n";
  }
}

void RustStructGenerator::Generate() {
  for (const auto &name : tree_.visit_order) {
    std::visit(*this, tree_.types.at(name));
  }
}

void RustStructGenerator::operator()(const ast::Message &ast_message) {
  output_ << "struct " << ast_message.identifier.name << " {" << std::endl;
  DeclareFields(ast_message.fields, 2);
  output_ << "}" << std::endl << std::endl;
}

void RustStructGenerator::operator()(const ast::Enum &ast_enum) {
  output_ << "enum " << ast_enum.identifier.name << " {" << std::endl;
  for (const auto &rule : ast_enum.pattern_mapping) {
    for (const auto &constructor : rule.outputs) {
      output_ << Tab(4) << constructor.identifier.name;
      const auto &fields = constructor.fields;
      if (!fields.empty()) {
        output_ << "{" << std::endl;
        DeclareFields(fields, 8);
        output_ << Tab(4) << "}";
      }
      output_ << "," << std::endl;
    }
  }
  output_ << "}" << std::endl << std::endl;
}

} // namespace dbuf::gen
