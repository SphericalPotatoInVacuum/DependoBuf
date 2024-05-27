#include "core/codegen/cpp_gen.h"

#include "glog/logging.h"

#include <fstream>
#include <sstream>
#include <unordered_set>

namespace dbuf::gen {

void CppCodeGenerator::PrintVariables(
    std::basic_ostream<char> &out,
    const std::vector<ast::TypedVariable> &variables,
    std::string &&delimeter,
    bool with_types,
    bool add_last_delimeter,
    bool as_dependency) {
  bool first = true;
  for (const auto &var : variables) {
    if (first) {
      first = false;
    } else {
      out << delimeter;
    }
    if (with_types) {
      (*this)(var, as_dependency);
    } else {
      out << var.name;
    }
  }
  if (add_last_delimeter && !first) {
    out << delimeter;
  }
}

bool CppCodeGenerator::CheckForTriggers(
    const std::unordered_set<InternedString> &trigger_names,
    const ast::Expression &expr) {
  bool result = false;
  if (std::holds_alternative<ast::BinaryExpression>(expr)) {
    result |= CheckForTriggers(trigger_names, *(std::get<ast::BinaryExpression>(expr).left)) ||
              CheckForTriggers(trigger_names, *(std::get<ast::BinaryExpression>(expr).right));
  } else if (std::holds_alternative<ast::UnaryExpression>(expr)) {
    result |= CheckForTriggers(trigger_names, *(std::get<ast::UnaryExpression>(expr).expression));
  } else if (std::holds_alternative<ast::Value>(expr)) {
    const auto &value = std::get<ast::Value>(expr);
    if (std::holds_alternative<ast::ConstructedValue>(value)) {
      const auto &constructed_value = std::get<ast::ConstructedValue>(value);
      if (std::holds_alternative<ast::Enum>(
              tree_->types.at(tree_->constructor_to_type.at(constructed_value.constructor_identifier.name)))) {
        return true;
      }
      for (const auto &[_, field] : constructed_value.fields) {
        result |= CheckForTriggers(trigger_names, *field);
      }
    }
  } else if (std::holds_alternative<ast::VarAccess>(expr)) {
    result |= trigger_names.contains(std::get<ast::VarAccess>(expr).var_identifier.name);
  }
  return result;
}

void CppCodeGenerator::Generate(const ast::AST *tree) {
  tree_ = tree;
  *output_ << "#include <string>\n";
  *output_ << "#include <variant>\n\n";
  *output_ << "namespace dbuf {\n";
  auto declaration_generator = [&](const auto &type) {
    if constexpr (std::is_same<std::decay_t<decltype(type)>, ast::Enum>()) {
      if (type.type_dependencies.size() != 0) {
        *output_ << "template <";
        PrintVariables(*output_, type.type_dependencies, ", ", true, false, true);
        *output_ << ">\n";
        *output_ << "struct " << type.identifier.name << ";\n\n";
      }
    }
  };

  for (const auto &type : tree->visit_order) {
    DLOG(INFO) << "Generating cpp struct" << type;
    std::visit(declaration_generator, tree->types.at(type));
    std::visit(*this, tree->types.at(type));
  }
  *output_ << "} // namespace dbuf\n";
}

void CppCodeGenerator::operator()(const ast::Message &ast_message) {
  const std::vector<ast::TypedVariable> checker_input;
  (*this)(ast_message, checker_input);
}

void CppCodeGenerator::operator()(
    const ast::Message &ast_message,
    const std::vector<ast::TypedVariable> &checker_input) {
  std::unordered_map<InternedString, std::vector<std::shared_ptr<const ast::Expression>>> checker_members;

  // Vector with final cpp fields for this message
  // To change Bar<a> to Bar_a without coping ast_message
  std::vector<ast::TypedVariable> cpp_struct_fields;
  cpp_struct_fields.reserve(ast_message.fields.size());

  // checker_input are variable dependencies of this type
  // they are known at runtime this is why they triggers new type creation
  // as other variables of message
  std::unordered_set<InternedString> trigger_names;
  for (const auto &var : checker_input) {
    trigger_names.insert(var.name);
  }

  // Generates all the needed extra types for this message
  // And fill cpp_struct_fields with relevant types
  for (const auto &field : ast_message.fields) {
    std::unordered_set<size_t> positions_of_variable_dependencies;
    // new field that will replace field in this message
    ast::TypedVariable new_field;
    new_field.name = field.name;
    for (size_t ind = 0; ind < field.type_expression.parameters.size(); ++ind) {
      if (CheckForTriggers(trigger_names, *field.type_expression.parameters[ind])) {
        positions_of_variable_dependencies.insert(ind);
      }
    }

    if (!positions_of_variable_dependencies.empty()) {
      // vector with expressions for this field in type check
      std::vector<std::shared_ptr<const ast::Expression>> variable_dependencies_expressions;

      const auto &previous_type = tree_->types.at(field.type_expression.identifier.name);
      std::vector<ast::TypedVariable> previous_type_dependencies;
      std::stringstream new_type_name;
      if (std::holds_alternative<ast::Message>(previous_type)) {
        previous_type_dependencies = std::get<ast::Message>(previous_type).type_dependencies;
        new_type_name << std::get<ast::Message>(previous_type).identifier.name;
      } else if (std::holds_alternative<ast::Enum>(previous_type)) {
        previous_type_dependencies = std::get<ast::Enum>(previous_type).type_dependencies;
        new_type_name << std::get<ast::Enum>(previous_type).identifier.name;
      }

      // filling vector expressions, getting new type name and constructing new field
      for (size_t ind = 0; ind < previous_type_dependencies.size(); ++ind) {
        if (positions_of_variable_dependencies.contains(ind)) {
          variable_dependencies_expressions.emplace_back(field.type_expression.parameters[ind]);
          new_type_name << "_" << previous_type_dependencies[ind].name;
        } else {
          new_field.type_expression.parameters.emplace_back(field.type_expression.parameters[ind]);
        }
      }

      // if such type was not already created - creating it
      if (!created_hidden_types_.contains(InternedString(new_type_name.str()))) {
        std::vector<ast::TypedVariable> hidden_dependencies;
        std::vector<ast::TypedVariable> real_dependencies;
        for (size_t ind = 0; ind < previous_type_dependencies.size(); ++ind) {
          if (!positions_of_variable_dependencies.contains(ind)) {
            real_dependencies.emplace_back(previous_type_dependencies[ind]);
          } else {
            hidden_dependencies.emplace_back(previous_type_dependencies[ind]);
          }
        }
        if (std::holds_alternative<ast::Message>(previous_type)) {
          DLOG(INFO) << "Generating cpp extra message " << new_type_name.str();
          ast::Message new_message;
          new_message.fields            = std::get<ast::Message>(previous_type).fields;
          new_message.type_dependencies = real_dependencies;
          new_message.identifier.name   = InternedString(new_type_name.str());

          (*this)(new_message, hidden_dependencies);
        } else if (std::holds_alternative<ast::Enum>(previous_type)) {
          DLOG(INFO) << "Generating cpp extra enum " << new_type_name.str();
          ast::Enum new_enum;
          new_enum.pattern_mapping   = std::get<ast::Enum>(previous_type).pattern_mapping;
          new_enum.type_dependencies = real_dependencies;
          new_enum.identifier.name   = InternedString(new_type_name.str());

          (*this)(new_enum, hidden_dependencies);
        }
        created_hidden_types_.insert(InternedString(new_type_name.str()));
      }

      checker_members[field.name] = std::move(variable_dependencies_expressions);

      new_field.type_expression.identifier.name = InternedString(new_type_name.str());
      cpp_struct_fields.emplace_back(new_field);
    } else {
      cpp_struct_fields.emplace_back(field);
    }

    trigger_names.insert(field.name);
  }

  DLOG(INFO) << "Generating cpp message " << ast_message.identifier.name << " templates";
  if (!ast_message.type_dependencies.empty()) {
    *output_ << "template <";
    PrintVariables(*output_, ast_message.type_dependencies, ", ", true, false, true);
    *output_ << ">\n";
  }

  *output_ << "struct " << ast_message.identifier.name << " {\n";

  DLOG(INFO) << "Generating cpp message " << ast_message.identifier.name << " static variables";
  for (auto &field : cpp_struct_fields) {
    for (auto &expr : field.type_expression.parameters) {
      if (std::holds_alternative<ast::Value>(*expr)) {
        const auto &value = std::get<ast::Value>(*expr);
        if (std::holds_alternative<ast::ScalarValue<std::string>>(value)) {
          std::string str = std::get<ast::ScalarValue<std::string>>(value).value;
          *output_ << "  constexpr static const char str_" << ++string_counter_ << "[] = \"" << str << "\";\n";
          ast::VarAccess new_expr;
          std::stringstream expr_name;
          expr_name << "str_" << string_counter_;
          new_expr.var_identifier.name = InternedString(expr_name.str());
          expr                         = std::make_shared<ast::Expression>(new_expr);
        }
      }
    }
  }

  DLOG(INFO) << "Generating cpp message " << ast_message.identifier.name << " fields";
  *output_ << "  ";
  PrintVariables(*output_, cpp_struct_fields, ";\n  ", true, true, false);

  DLOG(INFO) << "Generating cpp message " << ast_message.identifier.name << " invariant check";
  *output_ << "bool check(";
  PrintVariables(*output_, checker_input, ", ", true, false, false);
  *output_ << ") const {\n";

  *output_ << "    return true";
  for (const auto &[name, expressions] : checker_members) {
    *output_ << " && ";
    *output_ << name << ".check(";
    for (size_t ind = 0; ind < expressions.size(); ++ind) {
      std::visit(*this, *expressions[ind]);
      if (ind != expressions.size() - 1) {
        *output_ << ", ";
      }
    }
    *output_ << ")";
  }
  *output_ << ";\n  }\n";

  DLOG(INFO) << "Generating cpp message " << ast_message.identifier.name << " ending";
  *output_ << "};\n\n";
}

void CppCodeGenerator::operator()(const ast::TypedVariable &variable, bool as_dependency) {
  (*this)(variable.type_expression, as_dependency);
  *output_ << variable.name;
}

void CppCodeGenerator::operator()(const ast::TypeExpression &expr, bool as_dependency) {
  if (expr.identifier.name == InternedString("Int")) {
    *output_ << "int ";
  } else if (expr.identifier.name == InternedString("Unsigned")) {
    *output_ << "unsigned ";
  } else if (expr.identifier.name == InternedString("String")) {
    if (as_dependency) {
      *output_ << "const char *";
    } else {
      *output_ << "std::string ";
    }
  } else if (expr.identifier.name == InternedString("Float")) {
    *output_ << "double ";
  } else if (expr.identifier.name == InternedString("Bool")) {
    *output_ << "bool ";
  } else {
    if (as_dependency && std::holds_alternative<ast::Enum>(tree_->types.at(expr.identifier.name))) {
      *output_ << "const ";
    }
    *output_ << expr.identifier.name;
    for (size_t ind = 0; ind < expr.parameters.size(); ++ind) {
      if (ind == 0) {
        *output_ << "<";
      }
      std::visit(*this, *expr.parameters[ind]);
      if (ind == expr.parameters.size() - 1) {
        *output_ << ">";
      } else {
        *output_ << ", ";
      }
    }
    *output_ << " ";
    if (as_dependency && std::holds_alternative<ast::Enum>(tree_->types.at(expr.identifier.name))) {
      *output_ << "*";
    }
  }
}

void CppCodeGenerator::operator()(const ast::BinaryExpression &expr) {
  *output_ << "(";
  std::visit(*this, *expr.left);
  *output_ << " " << static_cast<char>(expr.type) << " ";
  std::visit(*this, *expr.right);
  *output_ << ")";
}

void CppCodeGenerator::operator()(const ast::UnaryExpression &expr) {
  *output_ << static_cast<char>(expr.type);
  std::visit(*this, *expr.expression);
}

void CppCodeGenerator::operator()(const ast::Value &value) {
  if (std::holds_alternative<ast::ConstructedValue>(value)) {
    std::visit(*this, value);
  } else {
    *output_ << value;
  }
}

void CppCodeGenerator::operator()(const ast::Star &star) {}

void CppCodeGenerator::operator()(const ast::VarAccess &var_access) {
  *output_ << var_access;
}

void CppCodeGenerator::operator()(const ast::ConstructedValue &value) {
  *output_ << value.constructor_identifier.name;
  *output_ << " {";
  for (size_t ind = 0; ind < value.fields.size(); ++ind) {
    std::visit(*this, *value.fields[ind].second);
    if (ind != value.fields.size() - 1) {
      *output_ << ", ";
    }
  }
  *output_ << "}";
}

void CppCodeGenerator::operator()(const ast::Enum &ast_enum) {
  bool has_all_star_case = false;

  auto print_complex_dependencies = [&](const ast::Enum::Rule &rule) {
    for (size_t ind = 0; (ind != ast_enum.type_dependencies.size()); ++ind) {
      if (ind == 0) {
        *output_ << "<";
      } else {
        *output_ << ", ";
      }

      if (!std::holds_alternative<ast::Star>(rule.inputs[ind])) {
        std::visit(*this, rule.inputs[ind]);
      } else {
        *output_ << ast_enum.type_dependencies[ind].name;
      }

      if (ind == ast_enum.type_dependencies.size() - 1) {
        *output_ << ">";
      }
    }
  };

  for (const auto &rule : ast_enum.pattern_mapping) {
    // As soon as i get *,*,* all following cases will create ill-formed struct
    if (has_all_star_case) {
      break;
    }

    // Divide dependencies in two parts
    std::vector<ast::TypedVariable> real_dependencies;
    std::vector<ast::Value> fixed_dependencies;
    bool all_stars = true;
    for (size_t ind = 0; ind < ast_enum.type_dependencies.size(); ++ind) {
      if (std::holds_alternative<ast::Star>(rule.inputs[ind])) {
        real_dependencies.emplace_back(ast_enum.type_dependencies[ind]);
      } else if (std::holds_alternative<ast::Value>(rule.inputs[ind])) {
        fixed_dependencies.emplace_back(std::get<ast::Value>(rule.inputs[ind]));
        all_stars = false;
      }
    }
    has_all_star_case |= all_stars;

    for (const auto &constructor : rule.outputs) {
      DLOG(INFO) << "Generating cpp enum " << ast_enum.identifier.name << " constructor "
                 << constructor.identifier.name;
      ast::Message sub_struct;

      sub_struct.type_dependencies = ast_enum.type_dependencies;
      sub_struct.identifier.name   = constructor.identifier.name;
      sub_struct.fields            = constructor.fields;

      (*this)(sub_struct);
    }

    DLOG(INFO) << "Generating cpp enum " << ast_enum.identifier.name << " dependencies";
    if (!ast_enum.type_dependencies.empty()) {
      *output_ << "template <";
      PrintVariables(*output_, real_dependencies, ", ", true, false, true);
      *output_ << ">\n";
    }

    DLOG(INFO) << "Generating cpp enum " << ast_enum.identifier.name << " name and input pattern";
    *output_ << "struct " << ast_enum.identifier.name;
    if (!all_stars) {
      print_complex_dependencies(rule);
    }
    *output_ << " {\n";

    DLOG(INFO) << "Generating cpp enum " << ast_enum.identifier.name << " variable";
    *output_ << "  std::variant<";
    bool first = true;
    for (const auto &rule_variant : rule.outputs) {
      if (first) {
        first = false;
      } else {
        *output_ << ", ";
      }
      *output_ << rule_variant.identifier.name;
      print_complex_dependencies(rule);
    }
    *output_ << "> value;\n";

    DLOG(INFO) << "Generating cpp enum " << ast_enum.identifier.name << " invariant check";
    *output_ << "  bool check() const {\n";
    *output_ << "    return false";
    first = true;
    for (const auto &constructor : rule.outputs) {
      *output_ << " ||";
      if (first) {
        first = false;
        *output_ << " ";
      } else {
        *output_ << "\n           ";
      }
      *output_ << "(std::holds_alternative<" << constructor.identifier.name;
      print_complex_dependencies(rule);
      *output_ << ">(value) && std::get<" << constructor.identifier.name;
      print_complex_dependencies(rule);
      *output_ << ">(value).check())";
    }
    *output_ << ";\n";
    *output_ << "  }\n";

    *output_ << "};\n\n";
  }

  DLOG(INFO) << "Generating cpp enum " << ast_enum.identifier.name << " extra empty specialisation";
  if (!has_all_star_case) {
    if (!ast_enum.type_dependencies.empty()) {
      *output_ << "template <";
      PrintVariables(*output_, ast_enum.type_dependencies, ", ", true, false, true);
      *output_ << ">\n";
    }
    *output_ << "struct " << ast_enum.identifier.name << " {\n";
    *output_ << "  bool check() const {\n";
    *output_ << "    return false;\n";
    *output_ << "  }\n";
    *output_ << "};\n\n";
  }
}

void CppCodeGenerator::operator()(const ast::Enum &ast_enum, const std::vector<ast::TypedVariable> &checker_input) {
  // Generates enum wirh runtime dependencies

  // I need original correctly ordered dependencied for each case at type check
  std::string original_name = ast_enum.identifier.name.GetString();
  std::string added_name    = original_name.substr(original_name.find_first_of('_'));
  original_name             = original_name.substr(0, original_name.find_first_of('_'));

  for (size_t ind = 0; ind < ast_enum.pattern_mapping.size(); ++ind) {
    for (const auto &constructor : ast_enum.pattern_mapping[ind].outputs) {
      ast::Message sub_struct;

      std::stringstream struct_name;
      struct_name << constructor.identifier.name << "_" << ind + 1 << added_name;
      DLOG(INFO) << "Generating cpp extra_enum " << ast_enum.identifier.name << " constructor " << struct_name.str();

      sub_struct.type_dependencies = ast_enum.type_dependencies;
      sub_struct.identifier.name   = InternedString(struct_name.str());
      sub_struct.fields            = constructor.fields;

      (*this)(sub_struct, checker_input);
    }
  }

  DLOG(INFO) << "Generating cpp extra_enum " << ast_enum.identifier.name << " templates";
  if (!ast_enum.type_dependencies.empty()) {
    *output_ << "template <";
    PrintVariables(*output_, ast_enum.type_dependencies, ", ", true, false, true);
    *output_ << ">\n";
  }

  *output_ << "struct " << ast_enum.identifier.name << " {\n";

  DLOG(INFO) << "Generating cpp extra_enum " << ast_enum.identifier.name << " variable";
  *output_ << "  std::variant<";
  bool first = true;
  for (size_t ind = 0; ind < ast_enum.pattern_mapping.size(); ++ind) {
    for (const auto &constructor : ast_enum.pattern_mapping[ind].outputs) {
      if (first) {
        first = false;
      } else {
        *output_ << ", ";
      }
      *output_ << constructor.identifier.name << "_" << ind + 1 << added_name;
      if (!ast_enum.type_dependencies.empty()) {
        *output_ << "<";
        PrintVariables(*output_, ast_enum.type_dependencies, ", ", false, false, true);
        *output_ << ">";
      }
    }
  }
  *output_ << "> value;\n";

  DLOG(INFO) << "Generating cpp extra_enum " << ast_enum.identifier.name << " invariant check";
  *output_ << "  bool check(";
  PrintVariables(*output_, checker_input, ", ", true, false, true);
  *output_ << ") const {\n";

  const auto &original_dependencies =
      std::get<ast::Enum>(tree_->types.at(InternedString(original_name))).type_dependencies;

  first = true;
  for (size_t ind = 0; ind < ast_enum.pattern_mapping.size(); ++ind) {
    if (first) {
      first = false;
      *output_ << "   ";
    } else {
      *output_ << "    else";
    }
    first = true;
    for (size_t input_ind = 0; input_ind < ast_enum.pattern_mapping[ind].inputs.size(); ++input_ind) {
      if (std::holds_alternative<ast::Star>(ast_enum.pattern_mapping[ind].inputs[input_ind])) {
        continue;
      }
      if (first) {
        first = false;
        *output_ << " if (";
      } else {
        *output_ << " && ";
      }
      *output_ << original_dependencies[input_ind].name << " == ";
      std::visit(*this, ast_enum.pattern_mapping[ind].inputs[input_ind]);
    }
    bool last_condition = first;
    *output_ << ((last_condition) ? "\n" : ")\n");
    *output_ << "      return ";

    first = true;
    for (const auto &constructor : ast_enum.pattern_mapping[ind].outputs) {
      std::stringstream constructor_name;
      constructor_name << constructor.identifier.name << "_" << ind + 1 << added_name;
      if (first) {
        first = false;
      } else {
        *output_ << " || ";
      }
      *output_ << "(std::holds_alternative<" << constructor_name.str();
      if (!ast_enum.type_dependencies.empty()) {
        *output_ << "<";
        PrintVariables(*output_, ast_enum.type_dependencies, ", ", false, false, true);
        *output_ << ">";
      }
      *output_ << ">(value) && std::get<" << constructor_name.str();
      if (!ast_enum.type_dependencies.empty()) {
        *output_ << "<";
        PrintVariables(*output_, ast_enum.type_dependencies, ", ", false, false, true);
        *output_ << ">";
      }
      *output_ << ">(value).check(";
      first = true;
      for (const auto &var : checker_input) {
        if (first) {
          first = false;
        } else {
          *output_ << ", ";
        }
        *output_ << var.name;
      }
      *output_ << "))";
    }
    *output_ << ";\n";

    // as soon as i got all stars no other cases are needed
    if (last_condition) {
      break;
    }
  }
  *output_ << "    return false;\n";
  *output_ << "  }\n";

  DLOG(INFO) << "Generating cpp extra_enum " << ast_enum.identifier.name << " ending";
  *output_ << "};\n\n";
}
} // namespace dbuf::gen
