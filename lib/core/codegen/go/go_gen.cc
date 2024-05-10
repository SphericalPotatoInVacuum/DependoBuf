#include "core/codegen/go/go_gen.h"

#include "core/ast/ast.h"
#include "core/ast/expression.h"
#include "core/interning/interned_string.h"

#include <iostream>

namespace dbuf::gen {

namespace {

static constexpr std::string_view kDoNotEdit          = "THIS FILE IS AUTOGENERATED, DO NOT EDIT IT";
static constexpr std::size_t kIndentLength            = 4;
static constexpr std::string_view kDefaultPackageName = "dbuf";

static const std::unordered_map<std::string, std::string> kPrimitiveTypesMapping = {
    {"Int", "int"},
    {"Int8", "int8"},
    {"Int16", "int16"},
    {"Int32", "int32"},
    {"Int64", "int64"},
    {"Float16", "float32"},
    {"Float32", "float32"},
    {"Float64", "float64"},
    {"String", "string"},
    {"Bool", "bool"},
};

struct GoNamedType {
  std::string name;
  std::string type;
};

struct GoStatementBody {
  std::vector<std::string> expressions;
};

struct GoFunction {
  std::optional<GoNamedType> receiver;
  std::string name;
  std::vector<GoNamedType> generics;
  std::vector<GoNamedType> args;
  std::vector<std::string> return_types;
  std::optional<GoStatementBody> body;
};

struct GoInterface {
  std::string name;
  std::vector<GoFunction> functions;
};

struct GoStruct {
  std::string name;
  std::vector<GoNamedType> fields;
};

struct GoTypeConstraint {
  std::string name;
  std::vector<std::string> types;
};

using GoStatement = std::variant<GoStruct, GoInterface, GoFunction, GoTypeConstraint>;

std::string GetExportName(std::string field_name) {
  if (std::isalpha(field_name[0]) && std::islower(field_name[0])) {
    field_name[0] = std::toupper(field_name[0]);
  }
  return field_name;
}

std::string GetIndent(std::size_t count) {
  return std::string(count * kIndentLength, ' ');
}

class GoWriter {
public:
  GoWriter(std::shared_ptr<std::ofstream> output)
      : output_(output) {}

  void SaveDependencyNames(InternedString type_name, const ast::DependentType &value);

  template <typename T>
  GoWriter &Write(const T &value);

  template <typename T>
  GoWriter &Write(const ast::ScalarValue<T> &value);

private:
  std::shared_ptr<std::ofstream> output_;
  std::unordered_map<InternedString, std::vector<InternedString>> dependency_names_;
};

void GoWriter::SaveDependencyNames(InternedString type_name, const ast::DependentType &value) {
  std::vector<InternedString> names;
  names.reserve(value.type_dependencies.size());
  for (const auto &dependency : value.type_dependencies) {
    names.push_back(dependency.name);
  }
  dependency_names_[type_name] = std::move(names);
}

template <typename T>
GoWriter &GoWriter::Write(const T &value) {
  *output_ << value;
  return *this;
}

template <>
GoWriter &GoWriter::Write(const InternedString &value) {
  return Write(value.GetString());
}

template <typename T>
GoWriter &GoWriter::Write(const ast::ScalarValue<T> &value) {
  return Write(value.value);
}

template <>
GoWriter &GoWriter::Write(const ast::ScalarValue<std::string> &value) {
  return Write('"').Write(value.value).Write('"');
}

template <>
GoWriter &GoWriter::Write(const ast::Expression &value);

template <>
GoWriter &GoWriter::Write(const ast::BinaryExpression &value) {
  return Write("(")
      .Write(*value.left)
      .Write(" ")
      .Write(static_cast<char>(value.type))
      .Write(" ")
      .Write(*value.right)
      .Write(")");
}

template <>
GoWriter &GoWriter::Write(const ast::UnaryExpression &value) {
  return Write(static_cast<char>(value.type)).Write(*value.expression);
}

template <>
GoWriter &GoWriter::Write(const ast::TypeExpression &value) {
  Write(value.identifier.name);
  for (const auto &parameter : value.parameters) {
    Write(" ").Write(*parameter);
  }
  return *this;
}

template <>
GoWriter &GoWriter::Write(const ast::Value &value);

template <>
GoWriter &GoWriter::Write(const ast::VarAccess &value) {
  Write("obj.").Write(value.var_identifier.name);
  for (const auto &field : value.field_identifiers) {
    Write(".").Write(field.name);
  }
  return *this;
}

template <>
GoWriter &GoWriter::Write(const ast::ConstructedValue &value) {
  Write(value.constructor_identifier.name).Write("{");
  if (!value.fields.empty()) {
    Write('\n');
    bool is_first_field = true;
    for (const auto &[identifier, value] : value.fields) {
      if (is_first_field) {
        is_first_field = false;
      } else {
        Write(", ");
      }
      Write(identifier.name).Write(": ").Write(*value);
    }
    Write('\n');
  }
  return Write("}");
}

template <>
GoWriter &GoWriter::Write(const ast::Value &value) {
  std::visit([this](const auto &inner_value) { Write(inner_value); }, value);
  return *this;
}

template <>
GoWriter &GoWriter::Write(const ast::Expression &value) {
  std::visit([this](const auto &inner_value) { Write(inner_value); }, value);
  return *this;
}

template <>
GoWriter &GoWriter::Write(const ast::Message &msg) {
  std::cout << "generating message" << std::endl;
  // generate struct
  const auto &message_name = msg.identifier.name.GetString();
  Write('\n');
  Write("type ").Write(message_name).Write(" struct {").Write('\n');
  if (!msg.type_dependencies.empty()) {
    Write(GetIndent(1)).Write("// dependencies").Write('\n');
    for (const auto &dependency : msg.type_dependencies) {
      const auto &dependency_name = dependency.name.GetString();
      std::string dependency_type = dependency.type_expression.identifier.name.GetString();
      if (kPrimitiveTypesMapping.contains(dependency_type)) {
        dependency_type = kPrimitiveTypesMapping.at(dependency_type);
      } else {
        dependency_type = "*" + dependency_type;
      }
      Write(GetIndent(1)).Write(dependency_name).Write(' ').Write(dependency_type).Write('\n');
    }
    if (!msg.fields.empty()) {
      Write('\n');
    }
  }
  for (const auto &field : msg.fields) {
    const std::string field_name = GetExportName(field.name.GetString());
    std::string field_type       = field.type_expression.identifier.name.GetString();
    if (kPrimitiveTypesMapping.contains(field_type)) {
      field_type = kPrimitiveTypesMapping.at(field_type);
    } else {
      field_type = "*" + field_type;
    }
    Write(GetIndent(1)).Write(field_name).Write(' ').Write(field_type).Write('\n');
  }
  Write('}').Write('\n');
  Write('\n');
  // generate New
  Write("func New").Write(message_name).Write('(');
  if (!msg.type_dependencies.empty()) {
    bool is_first_argument = true;
    for (const auto &dependency : msg.type_dependencies) {
      const auto &dependency_name = dependency.name.GetString();
      std::string dependency_type = dependency.type_expression.identifier.name.GetString();
      if (kPrimitiveTypesMapping.contains(dependency_type)) {
        dependency_type = kPrimitiveTypesMapping.at(dependency_type);
      } else {
        dependency_type = "*" + dependency_type;
      }
      if (is_first_argument) {
        is_first_argument = false;
        Write(dependency_name).Write(' ').Write(dependency_type);
      } else {
        Write(", ").Write(dependency_name).Write(' ').Write(dependency_type);
      }
    }
  }
  Write(") *").Write(message_name).Write(" {").Write('\n');
  Write(GetIndent(1)).Write("return &").Write(message_name).Write("{");
  if (!msg.type_dependencies.empty()) {
    Write('\n');
    Write(GetIndent(2));
    for (const auto &dependency : msg.type_dependencies) {
      const auto &dependency_name = dependency.name.GetString();
      Write(dependency_name).Write(": ").Write(dependency_name).Write(", ");
    }
    Write('\n');
    Write(GetIndent(1)).Write('}').Write('\n');
  } else {
    Write('}').Write('\n');
  }
  Write('}').Write('\n');
  Write('\n');
  // generate Validate
  Write("func(obj *").Write(message_name).Write(") Validate() bool {");
  if (!msg.type_dependencies.empty() || !msg.fields.empty()) {
    Write('\n');
  }
  for (const auto &dependency : msg.type_dependencies) {
    if (dependency.type_expression.parameters.empty()) {
      continue;
    }
    const auto &dependency_type = dependency.type_expression.identifier.name.GetString();
    Write(GetIndent(1)).Write("if (");
    bool is_first_condition = true;
    for (const auto &type_parameter_ptr : dependency.type_expression.parameters) {
      if (is_first_condition) {
        is_first_condition = false;
      } else {
        Write(" || ");
      }
      Write("obj.").Write(dependency.name).Write(" != ");
      std::visit([this](const auto &parameter) { Write(parameter); }, *type_parameter_ptr);
    }
    Write(") {").Write('\n');
    Write(GetIndent(2)).Write("return false;").Write('\n');
    Write(GetIndent(1)).Write('}').Write('\n');
  }
  Write(GetIndent(1)).Write("return true;").Write('\n');
  Write('}').Write('\n');
  return *this;
}

template <>
GoWriter &GoWriter::Write(const ast::Enum &en) {
  std::cout << "generating enum" << std::endl;
  // generate struct
  const auto &enum_name = en.identifier.name.GetString();
  Write('\n');
  Write("type ").Write(enum_name).Write(" struct {").Write('\n');
  for (const auto &dependency : en.type_dependencies) {
    const std::string dependency_name = dependency.name.GetString();
    std::string dependency_type       = dependency.type_expression.identifier.name.GetString();
    if (kPrimitiveTypesMapping.contains(dependency_type)) {
      dependency_type = kPrimitiveTypesMapping.at(dependency_type);
    } else {
      dependency_type = "*" + dependency_type;
    }
    Write(GetIndent(1)).Write(dependency_name).Write(' ').Write(dependency_type).Write('\n');
  }
  Write('\n');
  Write(GetIndent(1)).Write("InternalValue interface{}").Write('\n');
  Write('}').Write('\n');
  Write('\n');
  // constructors
  std::vector<std::string> constructor_names;
  for (const auto &pattern : en.pattern_mapping) {
    for (const auto &constructor : pattern.outputs) {
      const auto &constructor_name = constructor.identifier.name.GetString();
      constructor_names.push_back(constructor_name);
      Write("type ").Write(constructor_name).Write(" struct {").Write('\n');
      for (const auto &field : constructor.fields) {
        const std::string field_name = GetExportName(field.name.GetString());
        std::string field_type       = field.type_expression.identifier.name.GetString();
        if (kPrimitiveTypesMapping.contains(field_type)) {
          field_type = kPrimitiveTypesMapping.at(field_type);
        } else {
          field_type = "*" + field_type;
        }
        Write(GetIndent(1)).Write(field_name).Write(' ').Write(field_type).Write('\n');
      }
      Write('}').Write('\n');
      Write('\n');
    }
  }
  // type constraint
  const std::string type_constraint_name = en.identifier.name.GetString() + "TypeConstraint";
  Write("type ").Write(type_constraint_name).Write(" interface {");
  if (!constructor_names.empty()) {
    Write('\n');
    Write(GetIndent(1));
  }
  bool is_first_name = true;
  for (const auto &constructor_name : constructor_names) {
    if (is_first_name) {
      is_first_name = false;
      Write(constructor_name);
    } else {
      Write(" | ").Write(constructor_name);
    }
  }
  Write('\n');
  Write('}').Write('\n');
  Write('\n');
  // generate New
  Write("func New").Write(enum_name).Write('(');
  if (!en.type_dependencies.empty()) {
    bool is_first_argument = true;
    for (const auto &dependency : en.type_dependencies) {
      const auto &dependency_name = dependency.name.GetString();
      std::string dependency_type = dependency.type_expression.identifier.name.GetString();
      if (kPrimitiveTypesMapping.contains(dependency_type)) {
        dependency_type = kPrimitiveTypesMapping.at(dependency_type);
      } else {
        dependency_type = "*" + dependency_type;
      }
      if (is_first_argument) {
        is_first_argument = false;
        Write(dependency_name).Write(' ').Write(dependency_type);
      } else {
        Write(", ").Write(dependency_name).Write(' ').Write(dependency_type);
      }
    }
  }
  Write(") *").Write(enum_name).Write(" {").Write('\n');
  Write(GetIndent(1)).Write("return &").Write(enum_name).Write('{');
  if (!en.type_dependencies.empty()) {
    Write('\n');
    Write(GetIndent(2));
    for (const auto &dependency : en.type_dependencies) {
      const auto &dependency_name = dependency.name.GetString();
      Write(dependency_name).Write(": ").Write(dependency_name).Write(", ");
    }
    Write('\n');
    Write(GetIndent(1)).Write('}').Write('\n');
  } else {
    Write('}').Write('\n');
  }
  Write('}').Write('\n');
  Write('\n');
  // SetValue
  Write("func SetValue")
      .Write(enum_name)
      .Write("[T ")
      .Write(type_constraint_name)
      .Write("](enum *")
      .Write(enum_name)
      .Write(", ")
      .Write("value *T) {")
      .Write('\n');
  Write(GetIndent(1)).Write("enum.InternalValue = value").Write('\n');
  Write('}').Write('\n');
  return *this;
}

} // namespace

void GoCodeGenerator::Generate(ast::AST *tree) {
  GoWriter writer(output_);

  writer.Write("package ").Write(kDefaultPackageName).Write('\n').Write('\n');
  writer.Write("// ").Write(kDoNotEdit).Write('\n');
  for (const auto &item : tree->visit_order) {
    const auto &variant = tree->types.at(item);
    std::visit([&writer, &item](const auto &entity) { writer.SaveDependencyNames(item, entity); }, variant);
  }
  for (const auto &item : tree->visit_order) {
    const auto &variant = tree->types.at(item);
    std::visit([&writer, &item](const auto &entity) { writer.Write(entity); }, variant);
  }
}

} // namespace dbuf::gen
