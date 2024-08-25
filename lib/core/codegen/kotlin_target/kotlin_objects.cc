#include "core/codegen/kotlin_target/kotlin_objects.h"

#include "core/codegen/kotlin_target/kotlin_error.h"

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
    {"Int", "0L"},
    {"Unsigned", "0UL"},
    {"String", "\"\""},
};

const std::string_view DependencyCheck::kErrorMessage = "dependency B of A (is ${A.B}) should be ${C}";

const std::string_view InitCheck::kErrorMessage = "property A should be initialized";

const std::string_view EnumRuleCheck::kErrorMessage = "not valid inside";

const std::string_view PrintableEnum::kPropertyName = "inside";

namespace {

std::string_view GetType(const dbuf::InternedString &type) {
  if (kTypeMap.find(type.GetString()) != kTypeMap.end()) {
    return kTypeMap.at(type.GetString());
  }
  return type.GetString();
}

/**
 * @brief Prints `ast::Value` to `printer`
 *
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
    const auto &current_value = std::get<ast::ConstructedValue>(value);
    printer << current_value.constructor_identifier.name << ".make(";
    for (size_t i = 0; i < current_value.fields.size(); ++i) {
      if (i != 0) {
        printer << ", ";
      }
      const auto &[field, expression] = current_value.fields[i];
      printer << field.name << " = " << PrintableExpression(*expression);
    }
    printer << ")";
  } else {
    throw KotlinError("unknow variant of ast::Expression::Value");
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
    const auto &dependency_type     = type.type_dependencies[i].type_expression.identifier;
    const auto &expression          = *property.type_expression.parameters[i];
    printer << DependencyCheck(dependency_name, dependency_property, dependency_type, expression);
    printer.NewLine();
  }
}

/**
 * @brief Prints all type checks for all `properties` and their dependent fields
 *
 */
void AddAllTypeChecks(
    Printer &printer,
    const std::vector<ast::TypedVariable> &properties,
    const ast::AST *tree,
    bool last = false) {
  bool printed = false;
  for (const auto &property : properties) {
    const auto &type = property.type_expression.identifier.name;
    if (kTypeMap.find(type.GetString()) != kTypeMap.end()) {
      continue;
    }
    if (property.type_expression.parameters.empty()) {
      continue;
    }
    printed                         = true;
    const auto &corresponded_object = tree->types.at(type);
    if (std::holds_alternative<ast::Message>(corresponded_object)) {
      AddTypeChecks(printer, property, std::get<ast::Message>(corresponded_object));
    } else if (std::holds_alternative<ast::Enum>(corresponded_object)) {
      AddTypeChecks(printer, property, std::get<ast::Enum>(corresponded_object));
    } else {
      throw KotlinError("unknow variant of ast.types.second");
    }
  }
  if (printed && !last) {
    printer.NewLine();
  }
}

/**
 * @brief Prints init checks for all `properties`
 *
 */
void AddAllInitChecks(Printer &printer, const std::vector<ast::TypedVariable> &properties, bool last = false) {
  bool printed = false;
  for (const auto &property : properties) {
    const auto &type = property.type_expression.identifier.name;
    if (kTypeMap.find(type.GetString()) != kTypeMap.end()) {
      continue;
    }
    printed = true;
    printer << InitCheck(property.name.GetString());
    printer.NewLine();
  }
  if (printed && !last) {
    printer.NewLine();
  }
}

} // namespace

ParenthesesScope::ParenthesesScope(Printer &printer)
    : Scope(printer) {
  Start();
}
void ParenthesesScope::Start() {
  printer_ << "(";
}
void ParenthesesScope::End() {
  printer_ << ")";
}
ParenthesesScope::~ParenthesesScope() {
  Close();
}

BracesScope::BracesScope(Printer &printer)
    : Scope(printer) {
  Start();
}
void BracesScope::Start() {
  printer_ << "{" << NewLine;
  printer_.AddIndent();
}
void BracesScope::End() {
  printer_.RemoveIndent();
  printer_ << "}";
  printer_ << NewLine;
}
BracesScope::~BracesScope() {
  Close();
}

PrintableVariable::PrintableVariable(const ast::TypedVariable &typed_variable)
    : typed_variable_(typed_variable) {}
void PrintableVariable::Print(Printer &printer) const {
  const auto &name = typed_variable_.name;
  const auto &type = typed_variable_.type_expression.identifier.name;
  printer << name << ": " << GetType(type);
}

Constructor::Constructor(const ast::DependentType &dependent_type)
    : dependent_type_(dependent_type) {}
void Constructor::Print(Printer &printer) const {
  ParenthesesScope scope(printer);
  SeparatablePrinter sprinter(printer, ", ");
  for (const auto &type : dependent_type_.type_dependencies) {
    sprinter << "val " << PrintableVariable(type) << Separate;
  }
}

DefaultTypeProperty::DefaultTypeProperty(const ast::TypedVariable &typed_variable)
    : typed_variable_(typed_variable) {}
void DefaultTypeProperty::Print(Printer &printer) const {
  const auto &type          = typed_variable_.type_expression.identifier.name;
  const auto &default_value = kDefaultValues.at(type.GetString());
  printer << "var " << PrintableVariable(typed_variable_) << " = " << default_value;
}

CustomTypeProperty::CustomTypeProperty(const ast::TypedVariable &typed_variable)
    : typed_variable_(typed_variable) {}
void CustomTypeProperty::Print(Printer &printer) const {
  printer << "lateinit var " << PrintableVariable(typed_variable_);
}

Property::Property(const ast::TypedVariable &typed_variable)
    : typed_variable_(typed_variable) {}
void Property::Print(Printer &printer) const {
  const auto &type = typed_variable_.type_expression.identifier.name;
  if (kTypeMap.find(type.GetString()) != kTypeMap.end()) {
    printer << DefaultTypeProperty(typed_variable_);
  } else {
    printer << CustomTypeProperty(typed_variable_);
  }
}

Properties::Properties(const ast::TypeWithFields &type_with_fields)
    : type_with_fields_(type_with_fields) {}
void Properties::Print(Printer &printer) const {
  if (type_with_fields_.fields.empty()) {
    return;
  }
  SeparatablePrinter sprinter(printer, NewLine);
  for (const auto &field : type_with_fields_.fields) {
    sprinter << Property(field) << Separate;
  }
  printer.NewLine();
  printer.NewLine();
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
    throw KotlinError("TypeExpressions are not supported");
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
    throw KotlinError("unknow variant of ast::Expression");
  }
}

SecondaryConstructor::SecondaryConstructor(const ast::TypeWithFields &fields, const ast::DependentType &dependency)
    : fields_(fields)
    , dependency_(dependency) {}
void SecondaryConstructor::Print(Printer &printer) const {
  printer << "@Throws(IllegalStateException::class) constructor";
  ParenthesesScope pscope1(printer);
  SeparatablePrinter sprinter1(printer, ", ");
  for (const auto &field : dependency_.type_dependencies) {
    sprinter1 << PrintableVariable(field) << Separate;
  }
  for (const auto &field : fields_.fields) {
    sprinter1 << PrintableVariable(field) << Separate;
  }
  if (fields_.fields.empty()) {
    sprinter1 << "@Suppress(\"UNUSED_PARAMETER\") _unused: Any" << Separate;
  }
  pscope1.Close();
  printer << " : this";
  ParenthesesScope pscope2(printer);
  SeparatablePrinter sprinter2(printer, ", ");
  for (const auto &field : dependency_.type_dependencies) {
    sprinter2 << field.name << Separate;
  }
  pscope2.Close();

  BracesScope bscope(printer);
  for (const auto &field : fields_.fields) {
    printer << "this." << field.name << " = " << field.name << NewLine;
  }
  if (!fields_.fields.empty()) {
    printer.NewLine();
  }
  printer << "check()" << NewLine;
}

SmartEqual::SmartEqual(const ast::Identifier &indentifiable, bool not_expression)
    : indentifiable_(indentifiable)
    , not_expression_(not_expression) {}
void SmartEqual::Print(Printer &printer) const {
  if (kTypeMap.find(indentifiable_.name.GetString()) != kTypeMap.end()) {
    if (not_expression_) {
      printer << " != ";
    } else {
      printer << " == ";
    }
  } else {
    if (not_expression_) {
      printer << " notSameFields ";
    } else {
      printer << " sameFields ";
    }
  }
}

DependencyCheck::DependencyCheck(
    const dbuf::InternedString &dependency_name,
    const dbuf::InternedString &dependency_property,
    const ast::Identifier &dependency_type,
    const ast::Expression &expression)
    : dependency_name_(dependency_name)
    , dependency_property_(dependency_property)
    , dependency_type_(dependency_type)
    , expression_(expression) {}
void DependencyCheck::Print(Printer &printer) const {
  printer << "check(" << dependency_name_ << "." << dependency_property_ << SmartEqual(dependency_type_)
          << PrintableExpression(expression_) << ") {\"";
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
  printer << "fun check() ";
  BracesScope scope(printer);

  AddAllTypeChecks(printer, message_.type_dependencies, tree_, message_.fields.empty());
  if (!message_.fields.empty()) {
    AddAllInitChecks(printer, message_.fields);
    AddAllTypeChecks(printer, message_.fields, tree_, true);
  }
}

ClassEquals::ClassEquals(
    const ast::Identifiable &target,
    const ast::TypeWithFields &fields,
    const ast::DependentType &dependency)
    : target_(target)
    , fields_(fields)
    , dependency_(dependency) {}
void ClassEquals::Print(Printer &printer) const {
  printer << "override fun equals(other: Any?): Boolean ";
  BracesScope bscope(printer);
  printer << "if (other !is " << target_.identifier.name << ") return false" << NewLine << NewLine;
  for (const auto &field : dependency_.type_dependencies) {
    printer << "if (this." << field.name << " != other." << field.name << ") return false" << NewLine;
  }
  if (!dependency_.type_dependencies.empty()) {
    printer.NewLine();
  }
  for (const auto &field : fields_.fields) {
    printer << "if (this." << field.name << " != other." << field.name << ") return false" << NewLine;
  }
  if (!fields_.fields.empty()) {
    printer.NewLine();
  }
  printer << "return true" << NewLine;
}

ClassSameFields::ClassSameFields(const ast::Identifiable &target, const ast::TypeWithFields &fields)
    : target_(target)
    , fields_(fields) {}
void ClassSameFields::Print(Printer &printer) const {
  printer << "infix internal fun sameFields(other: Any?): Boolean ";
  BracesScope bscope(printer);
  printer << "if (other !is " << target_.identifier.name << ") return false" << NewLine << NewLine;

  for (const auto &field : fields_.fields) {
    printer << "if (this." << field.name << SmartEqual(field.type_expression.identifier, true);
    printer << "other." << field.name << ") return false" << NewLine;
  }
  if (!fields_.fields.empty()) {
    printer.NewLine();
  }
  printer << "return true" << NewLine;
}

void ClassNotSameFields::Print(Printer &printer) const {
  printer << "infix internal fun notSameFields(other: Any?): Boolean ";
  BracesScope bscope(printer);
  printer << "return !(this sameFields other)" << NewLine;
}

void ClassToString::Print(Printer &printer) const {
  printer << "override fun toString(): String ";
  BracesScope bscope(printer);
  printer << "return toString(1U)" << NewLine;
}

ClassToStringImpl::ClassToStringImpl(
    const ast::Identifiable &target,
    const ast::TypeWithFields &fields,
    const ast::DependentType &dependency)
    : target_(target)
    , fields_(fields)
    , dependency_(dependency) {}
void ClassToStringImpl::Print(Printer &printer) const {
  printer << "fun toString(depth: UInt): String ";
  BracesScope bscope(printer);
  printer << "if (depth == 0U) return \"" << Printer::kPackageName << "." << target_.identifier.name << "@";
  printer << "${hashCode().toString(radix=16)}\"" << NewLine;
  printer << "return \"" << target_.identifier.name;
  if (!dependency_.type_dependencies.empty()) {
    printer << " <";
    SeparatablePrinter sprinter(printer, ", ");
    for (const auto &field : dependency_.type_dependencies) {
      sprinter << field.name << " = ";
      PrintValue(sprinter, field);
      sprinter << Separate;
    }
    printer << ">";
  }
  printer << " {";
  SeparatablePrinter sprinter(printer, ", ");
  for (const auto &field : fields_.fields) {
    sprinter << field.name << ": ";
    PrintValue(sprinter, field);
    sprinter << Separate;
  }
  printer << "}\"" << NewLine;
}
void ClassToStringImpl::PrintValue(SeparatablePrinter<const char *> &sprinter, const ast::TypedVariable &variable) {
  if (kTypeMap.find(variable.type_expression.identifier.name.GetString()) != kTypeMap.end()) {
    sprinter << "$" << variable.name;
  } else {
    sprinter << "${" << variable.name << ".toString(depth-1U)}";
  }
}

DefaultValue::DefaultValue(const ast::TypedVariable &variable)
    : variable_(variable) {}
void DefaultValue::Print(Printer &printer) const {
  const auto &type = variable_.type_expression.identifier.name;
  if (kDefaultValues.find(type.GetString()) != kDefaultValues.end()) {
    printer << kDefaultValues.at(type.GetString());
  } else {
    printer << type << ".default()";
  }
}

DefaultMessageCompanion::DefaultMessageCompanion(const ast::Message &message)
    : message_(message) {}
void DefaultMessageCompanion::Print(Printer &printer) const {
  printer << "fun default() : " << message_.identifier.name << " ";
  BracesScope bscope(printer);

  printer << "var return_object = ";
  printer << message_.identifier.name;
  ParenthesesScope pscope(printer);
  SeparatablePrinter sprinter(printer, ", ");
  for (const auto &dependency : message_.type_dependencies) {
    sprinter << DefaultValue(dependency) << Separate;
  }
  pscope.Close();
  printer.NewLine();

  printer << "return return_object" << NewLine;
}

MakeMessageCompanion::MakeMessageCompanion(const ast::Message &message)
    : message_(message) {}
void MakeMessageCompanion::Print(Printer &printer) const {
  printer << "fun make";
  ParenthesesScope pscope(printer);
  SeparatablePrinter sprinter(printer, ", ");
  for (const auto &field : message_.fields) {
    sprinter << PrintableVariable(field) << Separate;
  }
  pscope.Close();
  printer << ": " << message_.identifier.name << " ";
  BracesScope bscope(printer);

  printer << "var return_object = " << message_.identifier.name << ".default()" << NewLine;
  for (const auto &field : message_.fields) {
    printer << "return_object." << field.name << " = " << field.name << NewLine;
  }
  printer << "return return_object";
  printer.NewLine();
}

MessageCompanion::MessageCompanion(const ast::Message &message)
    : message_(message) {}
void MessageCompanion::Print(Printer &printer) const {
  printer << "internal companion object Factory ";
  BracesScope scope(printer);
  printer << DefaultMessageCompanion(message_);
  printer << MakeMessageCompanion(message_);
}

PrintableMessage::PrintableMessage(const ast::Message &message, const ast::AST *tree)
    : message_(message)
    , tree_(tree) {}
void PrintableMessage::Print(Printer &printer) const {
  printer << "class " << message_.identifier.name << Constructor(message_) << " ";
  BracesScope scope(printer);
  printer << Properties(message_);
  printer << SecondaryConstructor(message_, message_) << NewLine;
  printer << MessageCheck(message_, tree_) << NewLine;
  printer << ClassEquals(message_, message_, message_) << NewLine;
  printer << ClassToString() << NewLine;
  printer << ClassToStringImpl(message_, message_, message_) << NewLine;
  printer << ClassSameFields(message_, message_) << NewLine;
  printer << ClassNotSameFields() << NewLine;
  printer << MessageCompanion(message_);
  scope.Close();
  printer.NewLine();
}

ConstructorCheck::ConstructorCheck(const ast::Constructor &constructor, const ast::AST *tree)
    : constructor_(constructor)
    , tree_(tree) {}
void ConstructorCheck::Print(Printer &printer) const {
  printer << "fun check() ";
  BracesScope scope(printer);
  AddAllInitChecks(printer, constructor_.fields);
  AddAllTypeChecks(printer, constructor_.fields, tree_, true);
}

DefaultConstructorCompanion::DefaultConstructorCompanion(
    const ast::Constructor &constructor,
    const ast::DependentType &dependencies)
    : constructor_(constructor)
    , dependencies_(dependencies) {}
void DefaultConstructorCompanion::Print(Printer &printer) const {
  printer << "fun default() : " << constructor_.identifier.name << " ";
  BracesScope bscope(printer);

  printer << "var return_object = ";
  printer << constructor_.identifier.name;
  ParenthesesScope pscope(printer);
  SeparatablePrinter sprinter(printer, ", ");
  for (const auto &dependency : dependencies_.type_dependencies) {
    sprinter << DefaultValue(dependency) << Separate;
  }
  pscope.Close();
  printer.NewLine();

  printer << "return return_object" << NewLine;
}

MakeConstructorCompanion::MakeConstructorCompanion(
    const ast::Constructor &constructor,
    const dbuf::InternedString &enum_name)
    : constructor_(constructor)
    , enum_name_(enum_name) {}
void MakeConstructorCompanion::Print(Printer &printer) const {
  printer << "fun make";
  ParenthesesScope pscope(printer);
  SeparatablePrinter sprinter(printer, ", ");
  for (const auto &field : constructor_.fields) {
    sprinter << PrintableVariable(field) << Separate;
  }
  pscope.Close();
  printer << ": " << enum_name_ << " ";
  BracesScope bscope(printer);

  printer << "var return_object = " << enum_name_ << ".default()" << NewLine;
  printer << "var inside_object = " << constructor_.identifier.name << ".default()" << NewLine;
  for (const auto &field : constructor_.fields) {
    printer << "inside_object." << field.name << " = " << field.name << NewLine;
  }
  printer << "return_object." << PrintableEnum::kPropertyName << " = inside_object" << NewLine;
  printer << "return return_object" << NewLine;
}

ConstructorCompanion::ConstructorCompanion(
    const ast::Constructor &constructor,
    const ast::DependentType &dependencies,
    const dbuf::InternedString &enum_name)
    : constructor_(constructor)
    , dependencies_(dependencies)
    , enum_name_(enum_name) {}
void ConstructorCompanion::Print(Printer &printer) const {
  printer << "internal companion object Factory ";
  BracesScope scope(printer);
  printer << DefaultConstructorCompanion(constructor_, dependencies_);
  printer << MakeConstructorCompanion(constructor_, enum_name_);
}

PrintableConstructor::PrintableConstructor(
    const ast::DependentType &dependent_type,
    const ast::Constructor &constructor,
    const ast::AST *tree)
    : dependent_type_(dependent_type)
    , constructor_(constructor)
    , tree_(tree) {}
void PrintableConstructor::Print(Printer &printer) const {
  printer << "class " << constructor_.identifier.name << Constructor(dependent_type_) << " ";
  BracesScope scope(printer);
  printer << Properties(constructor_);
  printer << SecondaryConstructor(constructor_, dependent_type_) << NewLine;
  printer << ConstructorCheck(constructor_, tree_) << NewLine;
  printer << ClassEquals(constructor_, constructor_, dependent_type_) << NewLine;
  printer << ClassToString() << NewLine;
  printer << ClassToStringImpl(constructor_, constructor_, dependent_type_) << NewLine;
  printer << ClassSameFields(constructor_, constructor_) << NewLine;
  printer << ClassNotSameFields() << NewLine;
  printer << ConstructorCompanion(
      constructor_,
      dependent_type_,
      tree_->constructor_to_type.at(constructor_.identifier.name));
  scope.Close();
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
  printer << "if ";
  ParenthesesScope pscope(printer);
  SeparatablePrinter sprinter(printer, " && ");

  if (dependent_type_.type_dependencies.size() != rule_.inputs.size()) {
    throw KotlinError("bad rule for enum");
  }
  for (size_t i = 0; i < rule_.inputs.size(); ++i) {
    const auto &pattern = rule_.inputs[i];
    if (std::holds_alternative<ast::Star>(pattern)) {
      continue;
    }
    if (std::holds_alternative<ast::Value>(pattern)) {
      sprinter << EnumStatement(dependent_type_.type_dependencies[i], std::get<ast::Value>(pattern)) << Separate;
    } else {
      throw KotlinError("unknow variant of ast::Enum::InputPattern");
    }
  }
  if (!sprinter.NeedSeparator()) {
    printer << "true";
  }
  pscope.Close();
  printer << " ";
  BracesScope bscope(printer);
  if (rule_.outputs.empty()) {
    printer << "check(false) {\"" << kErrorMessage << "\"}";
    printer.NewLine();
  } else {
    const auto &property = PrintableEnum::kPropertyName;
    SeparatablePrinter sprinter(printer, "else ");
    for (const auto &constructor : rule_.outputs) {
      sprinter << "if (" << property << " is " << constructor.identifier.name << ") ";
      sprinter << "(" << property << " as " << constructor.identifier.name << ").check()" << NewLine;
      sprinter << Separate;
    }
    printer << "else ";
    printer << "check(false) {\"" << kErrorMessage << "\"}" << NewLine;
  }
  printer << "return" << NewLine;
  bscope.Close();
}

EnumCheck::EnumCheck(const ast::Enum &ast_enum, const ast::AST *tree)
    : ast_enum_(ast_enum)
    , tree_(tree) {}
void EnumCheck::Print(Printer &printer) const {
  printer << "fun check() ";
  BracesScope scope(printer);
  AddAllTypeChecks(printer, ast_enum_.type_dependencies, tree_);
  printer << InitCheck(PrintableEnum::kPropertyName);
  printer.NewLine();
  printer.NewLine();
  for (const auto &rule : ast_enum_.pattern_mapping) {
    printer << EnumRuleCheck(rule, ast_enum_);
  }
  printer << "check(false) {\"" << EnumRuleCheck::kErrorMessage << "\"}" << NewLine;
}

EnumSecondaryConstructor::EnumSecondaryConstructor(const ast::DependentType &dependency)
    : dependency_(dependency) {}
void EnumSecondaryConstructor::Print(Printer &printer) const {
  const auto &property = PrintableEnum::kPropertyName;

  printer << "@Throws(IllegalStateException::class) constructor";
  ParenthesesScope pscope1(printer);
  SeparatablePrinter sprinter1(printer, ", ");
  for (const auto &field : dependency_.type_dependencies) {
    sprinter1 << PrintableVariable(field) << Separate;
  }
  sprinter1 << property << ": Any" << Separate;
  pscope1.Close();
  printer << " : this";
  ParenthesesScope pscope2(printer);
  SeparatablePrinter sprinter2(printer, ", ");
  for (const auto &field : dependency_.type_dependencies) {
    sprinter2 << field.name << Separate;
  }
  pscope2.Close();
  printer << " ";
  BracesScope bscope(printer);
  printer << "this." << property << " = " << property << NewLine;
  printer << "check()" << NewLine;
}

EnumEquals::EnumEquals(const ast::Enum &ast_enum)
    : ast_enum_(ast_enum) {}
void EnumEquals::Print(Printer &printer) const {
  const auto &type      = ast_enum_.identifier.name;
  const auto &property  = PrintableEnum::kPropertyName;
  const auto &error_msg = EnumRuleCheck::kErrorMessage;

  printer << "override fun equals(other: Any?): Boolean ";
  BracesScope bscope(printer);
  printer << "if (other !is " << type << ") return false" << NewLine << NewLine;

  for (const auto &rule : ast_enum_.pattern_mapping) {
    for (const auto &ctr : rule.outputs) {
      printer << "if (" << property << " is " << ctr.identifier.name << ") return ";
      printer << "(" << property << " as " << ctr.identifier.name << ").equals(other." << property << ")" << NewLine;
    }
  }
  printer << NewLine;
  printer << "check(false) {\"" << error_msg << "\"}" << NewLine;
  printer << "return false" << NewLine;
}

EnumToStringImpl::EnumToStringImpl(const ast::Enum &ast_enum)
    : ast_enum_(ast_enum) {}
void EnumToStringImpl::Print(Printer &printer) const {
  const auto &type      = ast_enum_.identifier.name;
  const auto &property  = PrintableEnum::kPropertyName;
  const auto &error_msg = EnumRuleCheck::kErrorMessage;

  printer << "fun toString(depth: UInt): String ";
  BracesScope bscope(printer);
  printer << "if (depth == 0U) return \"" << Printer::kPackageName << "." << ast_enum_.identifier.name << "@";
  printer << "${hashCode().toString(radix=16)}\"" << NewLine << NewLine;

  for (const auto &rule : ast_enum_.pattern_mapping) {
    for (const auto &ctr : rule.outputs) {
      printer << "if (" << property << " is " << ctr.identifier.name << ") return \"(" << type << ") ${";
      printer << "(" << property << " as " << ctr.identifier.name << ").toString(depth)}\"" << NewLine;
    }
  }
  printer << NewLine;
  printer << "check(false) {\"" << error_msg << "\"}" << NewLine;
  printer << "return \"(" << type << ") Unknow\"" << NewLine;
}

EnumSameFields::EnumSameFields(const ast::Enum &ast_enum)
    : ast_enum_(ast_enum) {}
void EnumSameFields::Print(Printer &printer) const {
  const auto &type      = ast_enum_.identifier.name;
  const auto &property  = PrintableEnum::kPropertyName;
  const auto &error_msg = EnumRuleCheck::kErrorMessage;

  printer << "infix internal fun sameFields(other: Any?): Boolean ";
  BracesScope bscope(printer);
  printer << "if (other !is " << type << ") return false" << NewLine << NewLine;

  for (const auto &rule : ast_enum_.pattern_mapping) {
    for (const auto &ctr : rule.outputs) {
      printer << "if (" << property << " is " << ctr.identifier.name << ") return ";
      printer << "(" << property << " as " << ctr.identifier.name << ").sameFields(other." << property << ")"
              << NewLine;
    }
  }
  printer << NewLine;
  printer << "check(false) {\"" << error_msg << "\"}" << NewLine;
  printer << "return false" << NewLine;
}

DefaultEnumCompanion::DefaultEnumCompanion(const ast::Enum &ast_enum)
    : ast_enum_(ast_enum) {}
void DefaultEnumCompanion::Print(Printer &printer) const {
  printer << "fun default() : " << ast_enum_.identifier.name << " ";
  BracesScope bscope(printer);

  printer << "var return_object = ";
  printer << ast_enum_.identifier.name;
  ParenthesesScope pscope(printer);
  SeparatablePrinter sprinter(printer, ", ");
  for (const auto &dependency : ast_enum_.type_dependencies) {
    sprinter << DefaultValue(dependency) << Separate;
  }
  pscope.Close();
  printer.NewLine();

  printer << "return return_object" << NewLine;
}

EnumCompanion::EnumCompanion(const ast::Enum &ast_enum)
    : ast_enum_(ast_enum) {}
void EnumCompanion::Print(Printer &printer) const {
  printer << "internal companion object Factory ";
  BracesScope scope(printer);
  printer << DefaultEnumCompanion(ast_enum_);
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
  printer << "class " << ast_enum_.identifier.name << Constructor(ast_enum_) << " ";
  BracesScope scope(printer);
  printer << "lateinit var " << kPropertyName << ": Any" << NewLine << NewLine;
  printer << EnumSecondaryConstructor(ast_enum_) << NewLine;
  printer << EnumCheck(ast_enum_, tree_) << NewLine;
  printer << EnumEquals(ast_enum_) << NewLine;
  printer << ClassToString() << NewLine;
  printer << EnumToStringImpl(ast_enum_) << NewLine;
  printer << EnumSameFields(ast_enum_) << NewLine;
  printer << ClassNotSameFields() << NewLine;
  printer << EnumCompanion(ast_enum_);
  scope.Close();
  printer.NewLine();
}

} // namespace dbuf::gen::kotlin
