#include "core/codegen/kotlin_target/kotlin_objects.h"

#include <vector>

namespace dbuf::gen::kotlin {

const std::unordered_map<std::string_view, std::string_view> DefaultTypeProperty::kTypeMap = {
    {"Bool", "Boolean"},
    {"Float", "Double"},
    {"Int", "Long"},
    {"Unsigned", "ULong"},
    {"String", "String"},
};
const std::unordered_map<std::string_view, std::string_view> DefaultTypeProperty::kDefaultValues = {
    {"Bool", "false"},
    {"Float", "0.0"},
    {"Int", "0"},
    {"Unsigned", "0"},
    {"String", "\"\""},
};

const std::unordered_set<std::string_view> Properties::kDefaultTypes = {
    "Bool",
    "Float",
    "Int",
    "Unsigned",
    "String",
};

const std::string_view DependencyCheck::kErrorMessage = "dependency B of A (is ${A.B}) should be ${C}";

const std::string_view InitCheck::kErrorMessage = "property A should be initialized";

namespace {

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
    printer << current_value.value;
  } else if (std::holds_alternative<ast::ScalarValue<uint64_t>>(value)) {
    const auto &current_value = std::get<ast::ScalarValue<uint64_t>>(value);
    printer << current_value.value;
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

void AddTypeChecks(Printer &printer, const ast::TypedVariable &property, const ast::DependentType &type) {
  const auto &dependency_name = property.name;
  for (size_t i = 0; i < property.type_expression.parameters.size(); ++i) {
    const auto &dependency_property = type.type_dependencies[i].name;
    const auto &expression          = *property.type_expression.parameters[i];
    printer << DependencyCheck(dependency_name, dependency_property, expression);
    printer.NewLine();
  }
}

void AddAllTypeChecks(Printer &printer, const std::vector<ast::TypedVariable> &properties, const ast::AST *tree) {
  for (const auto &property : properties) {
    const auto &type = property.type_expression.identifier.name;
    if (Properties::kDefaultTypes.find(type.GetString()) != Properties::kDefaultTypes.end()) {
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

void AddAllInitChecks(Printer &printer, const std::vector<ast::TypedVariable> &properties) {
  for (const auto &property : properties) {
    const auto &type = property.type_expression.identifier.name;
    if (Properties::kDefaultTypes.find(type.GetString()) != Properties::kDefaultTypes.end()) {
      continue;
    }
    printer << InitCheck(property.name);
    printer.NewLine();
  }
}

} // namespace

ConstructorParameter::ConstructorParameter(const ast::TypedVariable &typed_variable)
    : typed_variable_(typed_variable) {}
void ConstructorParameter::Print(Printer &printer) const {
  const auto &name = typed_variable_.name;
  const auto &type = typed_variable_.type_expression.identifier.name;
  printer << "val " << name << ": " << type;
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
    if (kDefaultTypes.find(type) != kDefaultTypes.end()) {
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

InitCheck::InitCheck(const dbuf::InternedString &property)
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

PrintableMessage::PrintableMessage(const ast::Message &message, const ast::AST *tree)
    : message_(message)
    , tree_(tree) {}
void PrintableMessage::Print(Printer &printer) const {
  printer << "class " << message_.identifier.name << Constructor(message_) << " {";
  printer.NewLine();
  printer.AddIndent();
  printer << Properties(message_);
  printer.NewLine();

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
  printer.RemoveIndent();
  printer << "}";
  printer.NewLine();
}

} // namespace dbuf::gen::kotlin