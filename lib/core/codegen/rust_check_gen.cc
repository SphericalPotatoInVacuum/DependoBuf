#include "core/codegen/rust_check_gen.h"

#include "core/ast/ast.h"
#include "core/ast/expression.h"
#include "core/interning/interned_string.h"

#include <cstddef>
#include <string_view>
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

bool RustCheckGenerator::IsCopyableType(const InternedString &type) {
  return kBuiltinToArgumentType_.contains(type);
}

bool RustCheckGenerator::IsPrimitiveType(const InternedString &type) {
  if (type == InternedString("String")) {
    return true;
  }
  return IsCopyableType(type);
}

void RustCheckGenerator::DeclareCheckStart(const InternedString &name, const std::vector<ast::TypedVariable> &deps) {
  output_ << "impl " << name << " {\n";
  output_ << "    pub fn check(&self";
  for (const auto &dep : deps) {
    output_ << ", " << dep.name << ": ";
    const auto &type = dep.type_expression.identifier.name;
    if (IsCopyableType(type)) {
      output_ << kBuiltinToArgumentType_.at(type);
    } else {
      output_ << "&std::rc::Rc<" << type << '>';
    }
  }
  output_ << ") -> bool {\n";
}

void RustCheckGenerator::DeclareCheckEnd() {
  output_ << "        return true;\n";
  output_ << "    }\n";
  output_ << "}\n\n";
}

void RustCheckGenerator::PrintExpression(const ast::Expression &expr) {
  if (!std::holds_alternative<ast::Value>(expr)) {
    std::visit([this](const auto &expr) { output_ << expr; }, expr);
    return;
  }
  const auto &value = std::get<ast::Value>(expr);
  if (!std::holds_alternative<ast::ConstructedValue>(value)) {
    std::visit([this](const auto &value) { output_ << value; }, value);
    return;
  }
  const auto &constucted_value = std::get<ast::ConstructedValue>(value);
  PrintConstructedValue(constucted_value);
}

void RustCheckGenerator::PrintConstructedValue(const ast::ConstructedValue &expr) {
  output_ << "std::rc::Rc::new(" << expr.constructor_identifier.name << " { ";
  for (size_t i = 0; i < expr.fields.size(); ++i) {
    if (i != 0) {
      output_ << ", ";
    }
    const auto &[name, value] = expr.fields[i];
    output_ << name.name << ": (";
    PrintExpression(*value);
    output_ << ").clone()";
  }
  output_ << " })";
}

void RustCheckGenerator::CheckFields(const std::vector<ast::TypedVariable> &fields, const std::string_view &indent) {
  for (const auto &field : fields) {
    if (IsPrimitiveType(field.type_expression.identifier.name)) {
      continue;
    }
    output_ << indent << "if !" << field.name << ".check(";
    const auto &type      = field.type_expression;
    const auto &type_info = tree_.types.at(type.identifier.name);
    const auto &deps      = std::visit([](const auto &type) { return type.type_dependencies; }, type_info);
    // ^^^ That is absolutely insane
    for (size_t i = 0; i < type.parameters.size(); ++i) {
      if (i != 0) {
        output_ << ", ";
      }
      const auto &expr = *type.parameters[i];
      // vvv I beg this works
      if (!IsCopyableType(deps[i].type_expression.identifier.name)) {
        output_ << '&';
      }
      PrintExpression(expr);
    }
    output_ << ") { return false; }\n";
  }
}

void RustCheckGenerator::operator()(const ast::Message &ast_message) {
  DeclareCheckStart(ast_message.identifier.name, ast_message.type_dependencies);
  for (const auto &field : ast_message.fields) {
    output_ << "        let " << field.name << " = ";
    if (!IsCopyableType(field.type_expression.identifier.name)) {
      output_ << '&';
    }
    output_ << "self." << field.name << ";\n";
  }
  CheckFields(ast_message.fields, "        ");
  DeclareCheckEnd();
}

void RustCheckGenerator::operator()(const ast::Enum &ast_enum) {
  DeclareCheckStart(ast_enum.identifier.name, ast_enum.type_dependencies);

  output_ << "        match self {\n";
  for (const auto &rule : ast_enum.pattern_mapping) {
    for (const auto &constructor : rule.outputs) {
      output_ << "            " << constructor.identifier.name << " { ";
      for (size_t i = 0; i < constructor.fields.size(); ++i) {
        if (i != 0) {
          output_ << ", ";
        }
        const auto &field = constructor.fields[i];
        output_ << field.name;
      }
      output_ << " } => {\n";
      // for (size_t j = 0; j < rule.inputs.size(); ++j) {
      //   const auto &input = rule.inputs[j];
      //   if (std::holds_alternative<ast::Star>(input)) {
      //     continue;
      //   }
      //   const auto &value = std::get<ast::Value>(input);
      // }
      CheckFields(constructor.fields, "                ");
      output_ << "            }\n";
    }
  }
  output_ << "        }\n";

  DeclareCheckEnd();
}

} // namespace dbuf::gen