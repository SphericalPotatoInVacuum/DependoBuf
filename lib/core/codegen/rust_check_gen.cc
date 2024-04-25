#include "core/codegen/rust_check_gen.h"

#include "core/ast/ast.h"

#include <cstddef>
#include <variant>

namespace dbuf::gen {

RustCheckGenerator::RustCheckGenerator(std::ostream &output, const ast::AST &tree)
    : output_(output)
    , tree_(tree) {}

void RustCheckGenerator::Generate() {
  for (const auto &name : tree_.visit_order) {
    std::visit(*this, tree_.types.at(name));
  }
}

void RustCheckGenerator::operator()(const ast::Message &ast_message) {
  output_ << "impl " << ast_message.identifier.name << " {\n";
  output_ << "    fn check(&self";
  for (const auto &dep : ast_message.type_dependencies) {
    output_ << ", " << dep.name << ": ";
    const auto &type = dep.type_expression.identifier.name;
    if (kBuiltinToArgumentType_.contains(type)) {
      output_ << kBuiltinToArgumentType_.at(type);
    } else {
      output_ << '&' << type;
    }
  }
  output_ << ") {\n";

  for (const auto &field : ast_message.fields) {
    if (kBuiltinToArgumentType_.contains(field.type_expression.identifier.name)) {
      continue;
    }
    output_ << "        " << field.name << ".check(";
    const auto &type = field.type_expression;
    for (size_t i = 0; i < type.parameters.size(); ++i) {
      if (i != 0) {
        output_ << ", ";
      }
      const auto &expr = *type.parameters[i];
      output_ << expr;
    }
    output_ << ");\n";
  }
}

void RustCheckGenerator::operator()(const ast::Enum &ast_enum) {}

} // namespace dbuf::gen