#include "core/codegen/kotlin_target/kotlin_objects.h"

#include <vector>

namespace dbuf::gen::kotlin {

const std::unordered_map<std::string_view, std::string_view> kTypeMap = {
    {"Bool", "Boolean"},
    {"Float", "Double"},
    {"Int", "Long"},
    {"Unsigned", "ULong"},
    {"String", "String"},
};

const std::unordered_map<std::string_view, std::string_view> kDefaultValues = {
    {"Bool", "false"},
    {"Float", "0.0"},
    {"Int", "0"},
    {"Unsigned", "0"},
    {"String", "\"\""},
};

const std::string_view DependencyCheck::kErrorMessage = "dependency B of A (is ${A.B}) should be ${C}";

const std::string_view InitCheck::kErrorMessage = "property A should be initialized";

const std::string_view EnumRuleCheck::kErrorMessage = "not valid inside";

const std::string_view PrintableEnum::kPropertyName = "inside";

namespace {

/**
 * @brief Prints `ast::Value` to `printer`
 */
void PrintValue(Printer &printer, const ast::Value &value) {
  if (std::holds_alternative<ast::ScalarValue<bool>>(value)) {
    const auto &current_value = std::get<ast::ScalarValue<bool>>(value);
    if (current_value.value) {
      printer << "true";
    } else {
      printer << "false";
    }
  } else if (std::holds_alternative<ast::ScalarValue<double>>(value)) {
    const auto &current_value = std::get<ast::ScalarValue<double>>(value);
    printer << current_value.value;
  } else if (std::holds_alternative<ast::ScalarValue<int64_t>>(value)) {
    const auto &current_value = std::get<ast::ScalarValue<int64_t>>(value);
    printer << current_value.value << "L";
  } else if (std::holds_alternative<ast::ScalarValue<uint64_t>>(value)) {
    const auto &current_value = std::get<ast::ScalarValue<uint64_t>>(value);
    printer << current_value.value << "UL";
  } else if (std::holds_alternative<ast::ScalarValue<std::string>>(value)) {
    const auto &current_value = std::get<ast::ScalarValue<std::string>>(value);
    printer << "\"";
    for (const auto &ch : current_value.value) {
      if (ch == '\\') {
        printer << "\\\\";
      } else {
        printer << ch;
      }
    }
    printer << "\"";
  } else if (std::holds_alternative<ast::ConstructedValue>(value)) {
    throw std::string("kotlin code generation: ConstructedValue expression is not implemented");
  } else {
    throw std::string("kotlin code generation: unknow variant of ast::Expression::Value");
  }
}

/**
 * @brief Prints type cheks for all fields of `property`
 *
 */
void AddTypeChecks(Printer &printer, const ast::TypedVariable &property, const ast::DependentType &type) {
  const auto &dependency_name = property.name;
  for (size_t i = 0; i < property.type_expression.parameters.size(); ++i) {
    const auto &dependency_property = type.type_dependencies[i].name;
    const auto &expression          = *property.type_expression.parameters[i];
    printer << DependencyCheck(dependency_name, dependency_property, expression);
    printer.NewLine();
  }
}

/**
 * @brief Prints all type checks for all `properties` and their dependent fields
 *
 */
void AddAllTypeChecks(Printer &printer, const std::vector<ast::TypedVariable> &properties, const ast::AST *tree) {
  for (const auto &property : properties) {
    const auto &type = property.type_expression.identifier.name;
    if (kTypeMap.find(type.GetString()) != kTypeMap.end()) {
      continue;
    }
    const auto &corresponded_object = tree->types.at(type);
    if (std::holds_alternative<ast::Message>(corresponded_object)) {
      AddTypeChecks(printer, property, std::get<ast::Message>(corresponded_object));
    } else if (std::holds_alternative<ast::Enum>(corresponded_object)) {
      AddTypeChecks(printer, property, std::get<ast::Enum>(corresponded_object));
    } else {
      throw std::string("kotlin code generation: unknow variant of ast.types.second");
    }
  }
}

/**
 * @brief Prints init checks for all `properties`
 *
 */
void AddAllInitChecks(Printer &printer, const std::vector<ast::TypedVariable> &properties) {
  for (const auto &property : properties) {
    const auto &type = property.type_expression.identifier.name;
    if (kTypeMap.find(type.GetString()) != kTypeMap.end()) {
      continue;
    }
    printer << InitCheck(property.name.GetString());
    printer.NewLine();
  }
}

} // namespace

ConstructorParameter::ConstructorParameter(const ast::TypedVariable &typed_variable)
    : typed_variable_(typed_variable) {}
void ConstructorParameter::Print(Printer &printer) const {
  const auto &name = typed_variable_.name;
  const auto &type = typed_variable_.type_expression.identifier.name;
  printer << "val " << name << ": ";
  if (kTypeMap.find(type.GetString()) != kTypeMap.end()) {
    printer << kTypeMap.at(type.GetString());
  } else {
    printer << type;
  }
}

Constructor::Constructor(const ast::DependentType &dependent_type)
    : dependent_type_(dependent_type) {}
void Constructor::Print(Printer &printer) const {
  printer << "(";
  for (size_t i = 0; i < dependent_type_.type_dependencies.size(); ++i) {
    if (i != 0) {
      printer << ", ";
    }
    const auto &current = dependent_type_.type_dependencies[i];
    printer << ConstructorParameter(current);
  }
  printer << ")";
}

DefaultTypeProperty::DefaultTypeProperty(const ast::TypedVariable &typed_variable)
    : typed_variable_(typed_variable) {}
void DefaultTypeProperty::Print(Printer &printer) const {
  const auto &name          = typed_variable_.name;
  const auto &type          = typed_variable_.type_expression.identifier.name.GetString();
  const auto &print_type    = kTypeMap.at(type);
  const auto &default_value = kDefaultValues.at(type);
  printer << "var " << name << ": " << print_type << " = " << default_value;
}

CustomTypeProperty::CustomTypeProperty(const ast::TypedVariable &typed_variable)
    : typed_variable_(typed_variable) {}
void CustomTypeProperty::Print(Printer &printer) const {
  const auto &name = typed_variable_.name;
  const auto &type = typed_variable_.type_expression.identifier.name;
  printer << "lateinit var " << name << ": " << type;
}

Properties::Properties(const ast::TypeWithFields &type_with_fields)
    : type_with_fields_(type_with_fields) {}
void Properties::Print(Printer &printer) const {
  for (const auto &field : type_with_fields_.fields) {
    const auto &type = field.type_expression.identifier.name.GetString();
    if (kTypeMap.find(type) != kTypeMap.end()) {
      printer << DefaultTypeProperty(field);
    } else {
      printer << CustomTypeProperty(field);
    }
    printer.NewLine();
  }
}

PrintableExpression::PrintableExpression(const ast::Expression &expression)
    : expression_(expression) {}
void PrintableExpression::Print(Printer &printer) const {
  if (std::holds_alternative<ast::BinaryExpression>(expression_)) {
    const auto &current_expression = std::get<ast::BinaryExpression>(expression_);
    printer << "(" << PrintableExpression(*current_expression.left) << " " << static_cast<char>(current_expression.type)
            << " " << PrintableExpression(*current_expression.right) << ")";
  } else if (std::holds_alternative<ast::UnaryExpression>(expression_)) {
    const auto &current_expression = std::get<ast::UnaryExpression>(expression_);
    printer << static_cast<char>(current_expression.type) << "(" << PrintableExpression(*current_expression.expression)
            << ")";
  } else if (std::holds_alternative<ast::TypeExpression>(expression_)) {
    throw std::string("kotlin code generation: TypeExpressions are not supported");
  } else if (std::holds_alternative<ast::Value>(expression_)) {
    const auto &value = std::get<ast::Value>(expression_);
    PrintValue(printer, value);
  } else if (std::holds_alternative<ast::VarAccess>(expression_)) {
    const auto &current_expression = std::get<ast::VarAccess>(expression_);
    printer << current_expression.var_identifier.name;
    for (const auto &field : current_expression.field_identifiers) {
      printer << "." << field.name;
    }
  } else {
    throw std::string("kotlin code generation: unknow variant of ast::Expression");
  }
}

DependencyCheck::DependencyCheck(
    const dbuf::InternedString &dependency_name,
    const dbuf::InternedString &dependency_property,
    const ast::Expression &expression)
    : dependency_name_(dependency_name)
    , dependency_property_(dependency_property)
    , expression_(expression) {}
void DependencyCheck::Print(Printer &printer) const {
  printer << "check(" << dependency_name_ << "." << dependency_property_ << " == " << PrintableExpression(expression_)
          << ") {\"";
  for (const auto &ch : kErrorMessage) {
    if (ch == 'A') {
      printer << dependency_name_;
    } else if (ch == 'B') {
      printer << dependency_property_;
    } else if (ch == 'C') {
      printer << PrintableExpression(expression_);
    } else {
      printer << ch;
    }
  }
  printer << "\"}";
}

InitCheck::InitCheck(const std::string_view &property)
    : property_(property) {}
void InitCheck::Print(Printer &printer) const {
  printer << "check(this::" << property_ << ".isInitialized) {\"";
  for (const auto &ch : kErrorMessage) {
    if (ch == 'A') {
      printer << property_;
    } else {
      printer << ch;
    }
  }
  printer << "\"}";
}

MessageCheck::MessageCheck(const ast::Message &message, const ast::AST *tree)
    : message_(message)
    , tree_(tree) {}
void MessageCheck::Print(Printer &printer) const {
  printer << "fun check() {";
  printer.NewLine();
  printer.AddIndent();
  printer.StartPrintedCheck();
  AddAllTypeChecks(printer, message_.type_dependencies, tree_);
  if (printer.PrintedSomething()) {
    printer.NewLine();
  }
  printer.StartPrintedCheck();
  AddAllInitChecks(printer, message_.fields);
  if (printer.PrintedSomething()) {
    printer.NewLine();
  }
  AddAllTypeChecks(printer, message_.fields, tree_);
  printer.RemoveIndent();
  printer << "}";
  printer.NewLine();
}

PrintableMessage::PrintableMessage(const ast::Message &message, const ast::AST *tree)
    : message_(message)
    , tree_(tree) {}
void PrintableMessage::Print(Printer &printer) const {
  printer << "class " << message_.identifier.name << Constructor(message_) << " {";
  printer.NewLine();
  printer.AddIndent();
  if (!message_.fields.empty()) {
    printer << Properties(message_);
    printer.NewLine();
  }
  printer << MessageCheck(message_, tree_);
  printer.RemoveIndent();
  printer << "}";
  printer.NewLine();
  printer.NewLine();
}

ConstructorCheck::ConstructorCheck(const ast::Constructor &constructor, const ast::AST *tree)
    : constructor_(constructor)
    , tree_(tree) {}
void ConstructorCheck::Print(Printer &printer) const {
  printer << "fun check() {";
  printer.NewLine();
  printer.AddIndent();
  printer.StartPrintedCheck();
  AddAllInitChecks(printer, constructor_.fields);
  if (printer.PrintedSomething()) {
    printer.NewLine();
  }
  AddAllTypeChecks(printer, constructor_.fields, tree_);
  printer.RemoveIndent();
  printer << "}";
  printer.NewLine();
}

PrintableConstructor::PrintableConstructor(
    const ast::DependentType &dependent_type,
    const ast::Constructor &constructor,
    const ast::AST *tree)
    : dependent_type_(dependent_type)
    , constructor_(constructor)
    , tree_(tree) {}
void PrintableConstructor::Print(Printer &printer) const {
  printer << "class " << constructor_.identifier.name << Constructor(dependent_type_) << " {";
  printer.NewLine();
  printer.AddIndent();
  if (!constructor_.fields.empty()) {
    printer << Properties(constructor_);
    printer.NewLine();
  }
  printer << ConstructorCheck(constructor_, tree_);
  printer.RemoveIndent();
  printer << "}";
  printer.NewLine();
  printer.NewLine();
}

EnumStatement::EnumStatement(const ast::TypedVariable &target, const ast::Value &expect)
    : target_(target)
    , expect_(expect) {}
void EnumStatement::Print(Printer &printer) const {
  printer << target_.name << " == " << PrintableExpression(expect_);
}

EnumRuleCheck::EnumRuleCheck(const ast::Enum::Rule &rule, const ast::DependentType &dependent_type)
    : rule_(rule)
    , dependent_type_(dependent_type) {}
void EnumRuleCheck::Print(Printer &printer) const {
  printer << "if (";
  bool has_statement = false;
  if (dependent_type_.type_dependencies.size() != rule_.inputs.size()) {
    throw std::string("kotlin code generation: bad rule for enum");
  }
  for (size_t i = 0; i < rule_.inputs.size(); ++i) {
    const auto &pattern = rule_.inputs[i];
    if (std::holds_alternative<ast::Star>(pattern)) {
      continue;
    }
    if (std::holds_alternative<ast::Value>(pattern)) {
      if (has_statement) {
        printer << " && ";
      }
      printer << EnumStatement(dependent_type_.type_dependencies[i], std::get<ast::Value>(pattern));
      has_statement = true;
    } else {
      throw std::string("kotlin code generation: unknow variant of ast::Enum::InputPattern");
    }
  }
  if (!has_statement) {
    printer << "true";
  }
  printer << ") {";
  printer.NewLine();
  printer.AddIndent();
  if (rule_.outputs.empty()) {
    printer << "check(false) {\"" << kErrorMessage << "\"}";
    printer.NewLine();
  } else {
    const auto &property = PrintableEnum::kPropertyName;
    bool first           = true;
    for (const auto &constructor : rule_.outputs) {
      if (!first) {
        printer << "else ";
      }
      printer << "if (" << property << " is " << constructor.identifier.name << ") {";
      printer.NewLine();
      printer.AddIndent();
      printer << "(" << property << " as " << constructor.identifier.name << ").check()";
      printer.NewLine();
      printer.RemoveIndent();
      printer << "}";
      printer.NewLine();
      first = false;
    }
    printer << "else {";
    printer.NewLine();
    printer.AddIndent();
    printer << "check(false) {\"" << kErrorMessage << "\"}";
    printer.NewLine();
    printer.RemoveIndent();
    printer << "}";
    printer.NewLine();
  }
  printer << "return";
  printer.NewLine();
  printer.RemoveIndent();
  printer << "}";
  printer.NewLine();
}

EnumCheck::EnumCheck(const ast::Enum &ast_enum, const ast::AST *tree)
    : ast_enum_(ast_enum)
    , tree_(tree) {}
void EnumCheck::Print(Printer &printer) const {
  printer << "fun check() {";
  printer.NewLine();
  printer.AddIndent();
  printer.StartPrintedCheck();
  AddAllTypeChecks(printer, ast_enum_.type_dependencies, tree_);
  if (printer.PrintedSomething()) {
    printer.NewLine();
  }
  printer << InitCheck(PrintableEnum::kPropertyName);
  printer.NewLine();
  printer.NewLine();
  for (const auto &rule : ast_enum_.pattern_mapping) {
    printer << EnumRuleCheck(rule, ast_enum_);
  }
  printer.RemoveIndent();
  printer << "}";
  printer.NewLine();
}

PrintableEnum::PrintableEnum(const ast::Enum &ast_enum, const ast::AST *tree)
    : ast_enum_(ast_enum)
    , tree_(tree) {}
void PrintableEnum::Print(Printer &printer) const {
  for (const auto &rule : ast_enum_.pattern_mapping) {
    for (const auto &constructor : rule.outputs) {
      printer << PrintableConstructor(ast_enum_, constructor, tree_);
    }
  }
  printer << "class " << ast_enum_.identifier.name << Constructor(ast_enum_) << "{";
  printer.NewLine();
  printer.AddIndent();
  printer << "lateinit var " << kPropertyName << ": Any";
  printer.NewLine();
  printer.NewLine();
  printer << EnumCheck(ast_enum_, tree_);
  printer.RemoveIndent();
  printer << "}";
  printer.NewLine();
  printer.NewLine();
}

} // namespace dbuf::gen::kotlin
