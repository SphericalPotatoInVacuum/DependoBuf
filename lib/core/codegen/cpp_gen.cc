#include "core/codegen/cpp_gen.h"

#include <fstream>
#include <sstream>
#include <unordered_set>

namespace dbuf::gen {

void CppCodeGenerator::PrintVariables(
    std::basic_ostream<char> &out,
    std::vector<ast::TypedVariable> &variables,
    std::string delimeter,
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
      (*this)(var.type_expression, as_dependency);
      out << " ";
    }
    out << var.name;
  }
  if (add_last_delimeter) {
    out << delimeter;
  }
}

void CppCodeGenerator::Generate(const ast::AST *tree) {
  tree_ = tree;
  *output_ << "#include <string>\n";
  *output_ << "#include <variant>\n\n";
  *output_ << "namespace dbuf {\n";
  auto declaration_generator = [&](const auto &new_struct) {
    bool first = true;
    for (const auto &dependency : static_cast<ast::DependentType>(new_struct).type_dependencies) {
      if (first) {
        *output_ << "template <";
        first = false;
      } else {
        *output_ << ", ";
      }
      (*this)(dependency, true /* as_dependency */);
    }
    if (!first) {
      *output_ << ">\n";
    }
    *output_ << "struct " << static_cast<ast::Identifiable>(new_struct).identifier.name << ";\n\n";
  };

  for (const auto &type : tree->visit_order) {
    std::visit(declaration_generator, tree->types.at(type));
  }

  for (const auto &type : tree->visit_order) {
    std::visit(*this, tree->types.at(type));
  }
  *output_ << "}";
}

bool CheckForTriggers(const std::unordered_set<InternedString> &trigger_names, const ast::Expression &expr) {
  if (std::holds_alternative<ast::BinaryExpression>(expr)) {
    return CheckForTriggers(trigger_names, *(std::get<ast::BinaryExpression>(expr).left)) ||
           CheckForTriggers(trigger_names, *(std::get<ast::BinaryExpression>(expr).right));
  } else if (std::holds_alternative<ast::UnaryExpression>(expr)) {
    return CheckForTriggers(trigger_names, *(std::get<ast::UnaryExpression>(expr).expression));
  } else if (std::holds_alternative<ast::Value>(expr)) {
    const auto &value = std::get<ast::Value>(expr);
    if (std::holds_alternative<ast::ConstructedValue>(value)) {
      const auto &constructed_value = std::get<ast::ConstructedValue>(value);
      bool use_varibales            = false;
      for (const auto &[_, field] : constructed_value.fields) {
        use_varibales |= CheckForTriggers(trigger_names, *field);
      }
      return use_varibales;
    }
  } else if (std::holds_alternative<ast::VarAccess>(expr)) {
    return trigger_names.contains(std::get<ast::VarAccess>(expr).var_identifier.name);
  }
  return false;
}

void CppCodeGenerator::operator()(const ast::Message &ast_message, std::vector<ast::TypedVariable> checker_input) {
  std::unordered_map<InternedString, std::vector<std::shared_ptr<const ast::Expression>>> checker_members;

  // Vector with final cpp fields for this message
  // To change Bar<a> to Bar_a without coping ast_message to this func
  std::vector<ast::TypedVariable> cpp_struct_fields;
  cpp_struct_fields.reserve(ast_message.fields.size());

  // checker_input are variable dependencies of this type
  // they are known at runtime this is why they triggers new type creation
  // right as previously set variables
  std::unordered_set<InternedString> trigger_names;
  for (const auto &var : checker_input) {
    trigger_names.insert(var.name);
  }

  // Generates all the needed extra types for this message
  // And fill cpp_struct_fields with relevant types
  for (const auto &field : ast_message.fields) {
    std::unordered_set<size_t> positions_of_variable_dependencies;
    for (size_t ind = 0; ind < field.type_expression.parameters.size(); ++ind) {
      if (CheckForTriggers(trigger_names, *field.type_expression.parameters[ind])) {
        positions_of_variable_dependencies.insert(ind);
      }
    }

    if (!positions_of_variable_dependencies.empty()) {
      // vector of all the expressions for this field type check
      std::vector<std::shared_ptr<const ast::Expression>> variable_dependencies_expressions;

      auto previous_type                    = tree_->types.at(field.type_expression.identifier.name);
      const auto previous_type_dependencies = *reinterpret_cast<ast::DependentType *>(&previous_type);
      // new field that will replace field in this message
      ast::TypedVariable new_field;
      new_field.name = field.name;

      std::stringstream new_type_name;
      new_type_name << field.type_expression.identifier.name;

      // filling vector expressions, getting new type name and constructing new field
      for (size_t ind = 0; ind < previous_type_dependencies.type_dependencies.size(); ++ind) {
        if (positions_of_variable_dependencies.contains(ind)) {
          variable_dependencies_expressions.emplace_back(field.type_expression.parameters[ind]);
          new_message_name << "_" << previous_type_dependencies.type_dependencies[ind].name;
        } else {
          new_field.type_expression.parameters.emplace_back(field.type_expression.parameters[ind]);
        }
      }

      // if such type was not already created - creating it
      if (!created_hidden_types_.contains(InternedString(new_message_name.str()))) {
        ast::Message new_message;
        new_message.fields          = previous_type.fields;
        new_message.identifier.name = InternedString(new_message_name.str());

        std::vector<ast::TypedVariable> new_message_hidden_dependencies;
        for (size_t ind = 0; ind < previous_type.type_dependencies.size(); ++ind) {
          if (!positions_of_variable_dependencies.contains(ind)) {
            new_message.type_dependencies.emplace_back(previous_type.type_dependencies[ind]);
          } else {
            new_message_hidden_dependencies.emplace_back(previous_type.type_dependencies[ind]);
          }
        }

        created_hidden_types_.insert(new_message.identifier.name);
        (*this)(new_message, new_message_hidden_dependencies);
      }

      checker_members[field.name] = std::move(variable_dependencies_expressions);

      new_field.type_expression.identifier.name = InternedString(new_message_name.str());
      cpp_struct_fields.emplace_back(new_field);
    } else {
      cpp_struct_fields.emplace_back(field);
    }

    trigger_names.insert(field.name);
  }

  // Generate start (template <...> struct)

  for (size_t ind = 0; ind < ast_message.type_dependencies.size(); ++ind) {
    if (ind == 0) {
      *output_ << "template <";
    }

    (*this)(ast_message.type_dependencies[ind], true /* as dependency */);

    if (ind != ast_message.type_dependencies.size() - 1) {
      *output_ << ", ";
    } else {
      *output_ << ">\n";
    }
  }

  *output_ << "struct " << ast_message.identifier.name << " {\n";

  // Generate meessage fields

  for (const auto &field : cpp_struct_fields) {
    *output_ << "\t";
    (*this)(field, false /* as dependency */);
    *output_ << ";\n";
  }

  // Generate invariant check

  *output_ << "\n";
  *output_ << "\tbool check(";
  for (size_t ind = 0; ind < checker_input.size(); ++ind) {
    (*this)(checker_input[ind], false /* as_dependency */);
    if (ind != checker_input.size() - 1) {
      *output_ << ", ";
    }
  }
  *output_ << ") const {\n";

  // invariant check body
  *output_ << "\t\treturn true";
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
  *output_ << ";\n\t}\n";

  // Generate message end

  *output_ << "};\n\n";
}

void CppCodeGenerator::operator()(const ast::TypedVariable &expr, bool as_dependency) {
  (*this)(expr.type_expression, as_dependency);
  *output_ << " " << expr.name;
}

void CppCodeGenerator::operator()(const ast::TypeExpression &expr, bool as_dependency) {
  if (expr.identifier.name == InternedString("Int")) {
    *output_ << "int";
  } else if (expr.identifier.name == InternedString("Unsigned")) {
    *output_ << "unsigned";
  } else if (expr.identifier.name == InternedString("String")) {
    if (as_dependency) {
      *output_ << "const char*";
    } else {
      *output_ << "std::string";
    }
  } else if (expr.identifier.name == InternedString("Float")) {
    *output_ << "float";
  } else if (expr.identifier.name == InternedString("Bool")) {
    *output_ << "bool";
  } else {
    *output_ << expr.identifier.name.GetString();
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
  *output_ << "{";
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
  // Generation all his possible cases
  for (const auto &rule : ast_enum.pattern_mapping) {
    // Generated Dependencies
    *output_ << "template <";
    bool first = true;
    for (size_t ind = 0; ind != ast_enum.type_dependencies.size(); ++ind) {
      if (std::holds_alternative<ast::Star>(rule.inputs[ind])) {
        if (first) {
          first = false;
        } else {
          *output_ << ", ";
        }
        (*this)(ast_enum.type_dependencies[ind], true /* as_dependency */);
      }
    }
    *output_ << ">\n";

    // Generates name and input pattern
    *output_ << "struct " << ast_enum.identifier.name;
    for (size_t ind = 0; ind != ast_enum.type_dependencies.size(); ++ind) {
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

      if ((ind == ast_enum.type_dependencies.size() - 1) && !first) {
        *output_ << "> ";
      }
    }
    has_all_star_case |= (!first);
    *output_ << " {\n";

    // Declares constructors
    for (const auto &constructor : rule.outputs) {
      *output_ << "\tstruct " << constructor.identifier.name << ";\n";
    }

    *output_ << "\tstd::variant<";
    first = true;
    for (const auto &rule_variant : rule.outputs) {
      if (first) {
        first = false;
      } else {
        *output_ << ", ";
      }
      *output_ << rule_variant.identifier.name;
    }
    *output_ << "> value;\n";

    // Generates invariant check
    *output_ << "\tbool check() {\n";
    *output_ << "\t\treturn false";
    for (const auto &constructor : rule.outputs) {
      *output_ << " || ";
      *output_ << "(std::holds_alternative<" << constructor.identifier.name << ">(value) && std::get<"
               << constructor.identifier.name << ">(value).check())";
    }

    *output_ << "};\n\n";

    // Generates inner structs realisation
    for (const auto &constructor : rule.outputs) {
      ast::Message sub_struct;

      std::stringstream struct_name;
      struct_name << ast_enum.identifier.name;
      for (size_t ind = 0; ind != ast_enum.type_dependencies.size(); ++ind) {
        struct_name << ((ind == 0) ? "<" : ", ");

        if (!std::holds_alternative<ast::Star>(rule.inputs[ind])) {
          const auto &var = std::get<ast::Value>(rule.inputs[ind]);
          if (!std::holds_alternative<ast::ConstructedValue>(var)) {
            struct_name << var;
          } // else -> ?
        } else {
          struct_name << ast_enum.type_dependencies[ind].name;
          sub_struct.type_dependencies.push_back(ast_enum.type_dependencies[ind]);
        }

        if (ind == ast_enum.type_dependencies.size() - 1) {
          *output_ << "> ";
        }
      }
      sub_struct.fields = constructor.fields;
      (*this)(sub_struct);
    }
  }

  // Add empty specialisation with witch type will be complete
  if (!has_all_star_case) {
    for (size_t ind = 0; ind < ast_enum.type_dependencies.size(); ++ind) {
      *output_ << ((ind == 0) ? "template <" : ", ");
      (*this)(ast_enum.type_dependencies[ind], true /* as_dependency */);
      if (ind == ast_enum.type_dependencies.size() - 1) {
        *output_ << ">\n";
      }
    }
    *output_ << "struct " << ast_enum.identifier.name << " {};\n\n";
  }
}

void CppCodeGenerator::operator()(const ast::Enum &ast_enum, std::vector<ast::TypedVariable> &checker_input) {
  // Generates all remaining templates
  for (size_t ind = 0; ind < ast_enum.type_dependencies.size(); ++ind) {
    if (ind == 0) {
      *output_ << "template <";
    }

    (*this)(ast_enum.type_dependencies[ind], true /* as dependency */);

    if (ind != ast_enum.type_dependencies.size() - 1) {
      *output_ << ", ";
    } else {
      *output_ << ">\n";
    }
  }

  *output_ << "struct " << ast_enum.identifier.name << " {\n";

  // Declares all the constructors for all the rules
  for (size_t ind = 0; ind < ast_enum.pattern_mapping.size(); ++ind) {
    for (const auto &constructor : ast_enum.pattern_mapping[ind].outputs) {
      *output_ << "\tstruct " << constructor.identifier.name << "_" << ind + 1 << ";\n";
    }
  }

  // Generates main variable value
  *output_ << "\tstd::variant<";
  bool first = true;
  for (size_t ind = 0; ind < ast_enum.pattern_mapping.size(); ++ind) {
    for (const auto &constructor : ast_enum.pattern_mapping[ind].outputs) {
      if (first) {
        first = false;
      } else {
        *output_ << ", ";
      }
      *output_ << constructor.identifier.name << "_" << ind + 1;
    }
  }
  *output_ << "> value;\n";

  // Generates invariant check
  *output_ << "\tbool check(";

  first = true;
  for (const auto &var : checker_input) {
    if (first) {
      first = false;
    } else {
      *output_ << ", ";
    }
    (*this)(var, false /* as_dependency */);
  }
  *output_ << ") {\n";

  // I need original correctly ordered dependencied for each case at type check
  std::string original_name = ast_enum.identifier.name.GetString();
  original_name             = original_name.substr(0, original_name.find_first_of('_'));
  const auto &original_dependencies =
      std::get<ast::Enum>(tree_->types.at(InternedString(original_name))).type_dependencies;

  first = true;
  for (size_t ind = 0; ind < ast_enum.pattern_mapping.size(); ++ind) {
    if (first) {
      first = false;
      *output_ << "\t\t";
    } else {
      *output_ << "\t\telse ";
    }
    first = true;
    for (size_t input_ind = 0; input_ind < ast_enum.pattern_mapping[ind].inputs.size(); ++input_ind) {
      if (std::holds_alternative<ast::Star>(ast_enum.pattern_mapping[ind].inputs[input_ind])) {
        continue;
      }
      if (first) {
        first = false;
        *output_ << "if (";
      } else {
        *output_ << " && ";
      }
      *output_ << original_dependencies[input_ind].name << " == ";
      std::visit(*this, ast_enum.pattern_mapping[ind].inputs[input_ind]);
    }
    bool last_condition = first;
    *output_ << ((last_condition) ? "\n" : ")\n");
    *output_ << "\t\t\treturn ";

    first = true;
    for (const auto &constructor : ast_enum.pattern_mapping[ind].outputs) {
      std::stringstream constructor_name;
      constructor_name << constructor.identifier.name << "_" << ind;
      if (first) {
        first = false;
      } else {
        *output_ << " || ";
      }
      *output_ << "(std::holds_alternative<" << constructor_name.str() << ">(value) && std::get<"
               << constructor_name.str() << ">(value).check(";
      first = true;
      for (const auto &var : checker_input) {
        if (first) {
          first = false;
        } else {
          *output_ << ", ";
        }
        *output_ << var.name;
      }
      *output_ << ")";
    }
    *output_ << ";";

    // as soon as i got all stars no other cases are needed
    if (last_condition == true) {
      break;
    }
  }
  *output_ << "\t\treturn false;\n";
  *output_ << "\t}\n";
  *output_ << "};\n";

  // Generates all the declared messages
  for (size_t ind = 0; ind < ast_enum.pattern_mapping.size(); ++ind) {
    for (const auto &constructor : ast_enum.pattern_mapping[ind].outputs) {
      ast::Message sub_struct;
      std::stringstream struct_name;
      struct_name << ast_enum.identifier.name;
      for (size_t dependency_ind = 0; dependency_ind < ast_enum.type_dependencies.size(); dependency_ind++) {
        if (dependency_ind == 0) {
          struct_name << "<";
        } else {
          struct_name << ", ";
        }
        struct_name << ast_enum.type_dependencies[dependency_ind].name;
        if (dependency_ind == ast_enum.type_dependencies.size() - 1) {
          struct_name << ">";
        }
      }
      struct_name << "::" << constructor.identifier.name << "_" << ind;
      sub_struct.identifier.name = InternedString(struct_name.str());

      sub_struct.type_dependencies = ast_enum.type_dependencies;

      sub_struct.fields = constructor.fields;

      (*this)(sub_struct, checker_input);
    }
  }
}
} // namespace dbuf::gen
