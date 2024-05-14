#include "core/ast/ast.h"
#include "core/ast/expression.h"
#include "core/codegen/sharp_target/sharp_gen.h"
#include "core/codegen/sharp_target/sharp_print.h"
#include "core/interning/interned_string.h"

#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

namespace dbuf::gen {

std::size_t VectorHash::operator()(std::vector<std::pair<InternedString, InternedString>> &vec) const {
  std::size_t res = 0;
  std::hash<std::string> hash{};
  for (const auto& p : vec) {
    res = (res << 1) ^ hash(p.first.GetString());
    res = (res << 1) ^ hash(p.second.GetString());
  }
  return res;
}

std::size_t VectorHash::operator()(const std::vector<std::pair<InternedString, InternedString>> &vec) const {
    std::size_t res = 0;
    std::hash<std::string> hash{};
    for (const auto& p : vec) {
      res = (res << 1) ^ hash(p.first.GetString());
      res = (res << 1) ^ hash(p.second.GetString());
    }
    return res;
  }

SharpCodeGenerator::SharpCodeGenerator(const std::string &out_file)
    : ITargetCodeGenerator(out_file) {
  printer_ = SharpPrinter(output_);
};

void SharpCodeGenerator::Generate(const ast::AST *tree) {
  if (tree == nullptr) {
    std::cerr << "Sharp codegen did not get AST" << std::endl;
  } else {
    printer_.InitFile();

    tree_ = tree;

    for (const auto &type: tree->visit_order) {
      const std::variant<ast::Message, ast::Enum> &var = tree->types.at(type);
      if (std::holds_alternative<ast::Message>(var)) {
        const auto &dbuf_message = std::get<ast::Message>(var);
        (*this)(dbuf_message);
      } else {
        const auto &dbuf_enum = std::get<ast::Enum>(var);
        (*this)(dbuf_enum);
      }
    }

    printer_.CompleteFile();
  }
}

void SharpCodeGenerator::operator()(const ast::Message &ast_message) {
  const std::vector<ast::TypedVariable> checker_input;
  (*this)(ast_message, checker_input);
}

void SharpCodeGenerator::operator()(
  const ast::Message &ast_message,
  const std::vector<ast::TypedVariable> &checker_input) {
  const std::string &message_name = ast_message.identifier.name.GetString();

  // Vector with final sharp class fields for this message
  // To change Bar<a> to Bar_a without coping ast_message
  std::vector<ast::TypedVariable> class_fields;
  class_fields.reserve(ast_message.fields.size());

  // checker_input are variable dependencies of this type.
  // They are known at runtime this is why they triggers new type creation
  // as other variables of message
  std::unordered_set<InternedString> trigger_names;
  for (const auto &var : checker_input) {
    trigger_names.insert(var.name);
  }

  // Map contains pairs {checker_variable_name, checker_variable_expression}
  std::unordered_map<InternedString, std::vector<std::shared_ptr<const ast::Expression>>> checker_members;
  for (const auto &field : ast_message.fields) {
    // Set of positions of variable dependencies for checking checker triggers
    std::unordered_set<size_t> positions_of_variable_dependencies;

    for (size_t ind = 0; ind < field.type_expression.parameters.size(); ++ind) {
      if (CheckForTriggers(trigger_names, *field.type_expression.parameters[ind])) {
        positions_of_variable_dependencies.insert(ind);
      }
    }

    if (!positions_of_variable_dependencies.empty()) {
      const auto &previous_type = tree_->types.at(field.type_expression.identifier.name);
      std::vector<ast::TypedVariable> previous_type_dependencies;
      std::stringstream new_type_name;
      
      // New field that ast doen not contain
      ast::TypedVariable new_field;
      new_field.name = field.name;

      if (std::holds_alternative<ast::Message>(previous_type)) {
        previous_type_dependencies = std::get<ast::Message>(previous_type).type_dependencies;
        new_type_name << std::get<ast::Message>(previous_type).identifier.name;
      } else if (std::holds_alternative<ast::Enum>(previous_type)) {
        previous_type_dependencies = std::get<ast::Enum>(previous_type).type_dependencies;
        new_type_name << std::get<ast::Enum>(previous_type).identifier.name;
      } else {
        std::cerr << "Unknown dependency type in sharp code generation";
      }

      // Vector with expressions for this field in type check
      std::vector<std::shared_ptr<const ast::Expression>> variable_dependencies_expressions;

      for (size_t ind = 0; ind < previous_type_dependencies.size(); ++ind) {
        if (positions_of_variable_dependencies.contains(ind)) {
          variable_dependencies_expressions.emplace_back(field.type_expression.parameters[ind]);
          new_type_name << "_" << previous_type_dependencies[ind].name;
        } else {
          new_field.type_expression.parameters.emplace_back(field.type_expression.parameters[ind]);
        }
      }

      // If such type was not already created - creating it
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
          ast::Message new_message;
          new_message.fields            = std::get<ast::Message>(previous_type).fields;
          new_message.type_dependencies = real_dependencies;
          new_message.identifier.name   = InternedString(new_type_name.str());

          (*this)(new_message, hidden_dependencies);
        } else if (std::holds_alternative<ast::Enum>(previous_type)) {
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
      class_fields.emplace_back(new_field);
    } else {
      class_fields.emplace_back(field);
    }

    trigger_names.insert(field.name);
  }

  printer_.PrintClassBegin(message_name);

  // Vector contains pairs {dependent_variable_name, dependent_variable_type}
  std::vector<std::pair<InternedString, InternedString>> dependent_variables;
  dependent_variables.reserve(ast_message.type_dependencies.size());

  for (const auto &dependency : ast_message.type_dependencies) {
    (*this)(dependency, true);
    dependent_variables.emplace_back(dependency.name, dependency.type_expression.identifier.name);
  }

  PrintTypedVariables(class_fields, ";\n", true, true, false);

  printer_.PrintConstructorBegin(message_name, dependent_variables);

  for (auto &field : class_fields) {
    for (auto &expr : field.type_expression.parameters) {
      *output_ << "\t\t" << field.name << " = new " << field.type_expression.identifier.name << "(";
      (*this)(*expr);
      *output_ << ");\n";
    }
  }

  printer_.PrintConstructorEnd();

  PrintCheck(checker_members, checker_input);
  
  printer_.PrintClassEnd();
}

void SharpCodeGenerator::operator()(const ast::Enum &ast_enum) {
  const std::string &enum_name = ast_enum.identifier.name.GetString();

  std::vector<std::string> depentent_classes_names;
  std::unordered_map<std::string, std::vector<std::pair<InternedString, InternedString>>> dependent_variables;
  
  auto print_complex_dependencies_constructor = [&](
    const ast::Enum::Rule &rule,
    const std::vector<std::pair<InternedString, InternedString>> &dependent_variables) {
    for (size_t ind = 0; (ind != ast_enum.type_dependencies.size()); ++ind) {
      *output_ << " && " << dependent_variables[ind].first << "_" << " == ";
      const std::variant<ast::Value, ast::Star> &var = rule.inputs.at(ind);
      if (std::holds_alternative<ast::Value>(rule.inputs[ind])) {
        const auto &dbuf_value = std::get<ast::Value>(var);
        (*this)(dbuf_value);
      } else {
        *output_ << ast_enum.type_dependencies[ind].name;
      }
    } 
  };

  auto print_complex_dependencies_checker = [&](
    const ast::Enum::Rule &rule,
    const std::vector<std::pair<InternedString, InternedString>> &dependent_variables) {
    for (size_t ind = 0; (ind != ast_enum.type_dependencies.size()); ++ind) {
      *output_ << " && value." << dependent_variables[ind].first << " == ";
      const std::variant<ast::Value, ast::Star> &var = rule.inputs.at(ind);
      if (std::holds_alternative<ast::Value>(rule.inputs[ind])) {
        const auto &dbuf_value = std::get<ast::Value>(var);
        (*this)(dbuf_value);
      } else {
        *output_ << ast_enum.type_dependencies[ind].name;
      }
    }
  };
  
  bool has_all_star_case = false;
  for (const auto &rule : ast_enum.pattern_mapping) {
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

    std::vector<InternedString> inner_structs_names;
    inner_structs_names.reserve(rule.outputs.size());
    // Code generation for inner structs
    for (const auto & constructor : rule.outputs) {
       ast::Message sub_struct;

      sub_struct.type_dependencies = ast_enum.type_dependencies;
      sub_struct.identifier.name   = constructor.identifier.name;
      sub_struct.fields            = constructor.fields;

      (*this)(sub_struct);

      if(!ast_enum.type_dependencies.empty()) {
        inner_structs_names.push_back(sub_struct.identifier.name);
      }
    }

    std::string new_enum_name = enum_name;
    for (const auto &inner_structs_name : inner_structs_names) {
      new_enum_name += ("_" + inner_structs_name.GetString());
    }

    if (new_enum_name != enum_name) {
      depentent_classes_names.emplace_back(new_enum_name);
    }

    // Generates name and input pattern
    printer_.PrintClassBegin(new_enum_name);

    // Vector contains pairs {dependent_variable_name, dependent_variable_type}
    dependent_variables[new_enum_name].reserve(ast_enum.type_dependencies.size());

    for (const auto &dependency : ast_enum.type_dependencies) {
      dependent_variables[new_enum_name].emplace_back(dependency.name, dependency.type_expression.identifier.name);
    }

    if (!has_all_star_case) {
      *output_ << "\tprivate readonly bool IsConstructable;\n"
               << "\tpublic readonly Type[] Restrictions = {";
      for (size_t idx = 0; idx < inner_structs_names.size(); ++idx) {
        if (idx != 0) {
          *output_ << ", ";
        }
        *output_ << "typeof(" << inner_structs_names[idx] << ")";
      }
      *output_ << "};\n";

      printer_.PrintConstructorBegin(new_enum_name, dependent_variables[new_enum_name], false);
      *output_ << "\t\tIsConstructable = true";

      for (const auto &constructor : rule.outputs) {
        print_complex_dependencies_constructor(rule, dependent_variables[new_enum_name]);
      }
      *output_ << ";\n";

      printer_.PrintConstructorEnd();
      *output_ << "\tpublic bool Check() {\n"
               << "\t\treturn IsConstructable;\n"
               << "\t}\n";
    } else {
      for (const auto &dependency : ast_enum.type_dependencies) {
        (*this)(dependency, true);
      }
      *output_ << "\tdynamic value;\n";
      printer_.PrintConstructorBegin(new_enum_name, dependent_variables[new_enum_name]);
      printer_.PrintConstructorEnd();

      // Generates invariant check
      *output_ << "\tpublic bool Check() {\n"
               << "\t\treturn false";

      for (const auto &constructor : rule.outputs) {
        *output_ << " || "
                 << "(value is " << constructor.identifier.name << ")"
                 << " && "
                 << "value.Check()";
        print_complex_dependencies_checker(rule, dependent_variables[new_enum_name]);
      }
      *output_ << ";\n";
      *output_ << "\t}\n";
    }

    printer_.PrintClassEnd();
  }

  if (!has_all_star_case) {
    printer_.PrintClassBegin(enum_name);

    if (!depentent_classes_names.empty()) {
      *output_ << "\tprivate readonly Type[] AllowedTypes = {";
      for (size_t idx = 0; idx < depentent_classes_names.size(); ++idx) {
        if (idx != 0) {
          *output_ << ", ";
        }
        *output_ << "typeof(" << depentent_classes_names[idx] << ")";
      }
      *output_ << "};\n";
    }

    printer_.PrintBaseEnumFields();

    std::unordered_set<std::vector<std::pair<InternedString, InternedString>>, VectorHash> added_constructors;
    for (const auto &dependent_class_name : depentent_classes_names) {
      if (added_constructors.contains(dependent_variables[dependent_class_name])) {
        continue;
      }
      added_constructors.emplace(dependent_variables[dependent_class_name]);
      printer_.PrintConstructorBegin(enum_name, dependent_variables[dependent_class_name], false);
      printer_.PrintBaseEnumConstructor();
      printer_.PrintConstructorEnd();
    }

    printer_.PrintBaseEnumCheck();

    printer_.PrintClassEnd();
  }
}

void SharpCodeGenerator::operator()(
  [[maybe_unused]] const ast::Enum &ast_enum,
  [[maybe_unused]] const std::vector<ast::TypedVariable> &checker_input) {
  std::cerr << "Sharp code generation for enums with runtime dependencies is not emplemented yet" << std::endl;
}

void SharpCodeGenerator::operator()(const ast::TypedVariable &variable, bool as_dependency) {
  (*this)(variable.type_expression, as_dependency);
  *output_ << " " << variable.name << ";\n";
}

void SharpCodeGenerator::operator()(const ast::Expression &expr, bool as_dependency, bool need_access) {
  if (std::holds_alternative<ast::TypeExpression>(expr)) {
    const auto &type_expr = std::get<ast::TypeExpression>(expr);
    (*this)(type_expr, as_dependency, need_access);
  } else if (std::holds_alternative<ast::BinaryExpression>(expr)) {
    const auto &binary_expr = std::get<ast::BinaryExpression>(expr);
    (*this)(binary_expr);
  } else if (std::holds_alternative<ast::UnaryExpression>(expr)) {
    const auto &unary_expr = std::get<ast::UnaryExpression>(expr);
    (*this)(unary_expr);
  } else if (std::holds_alternative<ast::Value>(expr)) {
    const auto &val = std::get<ast::Value>(expr);
    (*this)(val);
  } else if (std::holds_alternative<ast::VarAccess>(expr)) {
    const auto &var_access = std::get<ast::VarAccess>(expr);
    (*this)(var_access);
  } else {
    std::cerr << "Unknown expression type in code generation" << std::endl;
  }
}

void SharpCodeGenerator::operator()(const ast::TypeExpression &expr, bool as_dependency, bool need_access) {
  printer_.PrintTypeExpression(expr, true, as_dependency, need_access);
}

void SharpCodeGenerator::operator()(const ast::BinaryExpression &expr) {
  printer_.PrintBinaryExpressionBegin();
  (*this)(*expr.left);
  printer_.PrintBinaryExpressionType(expr);
  (*this)(*expr.right);
  printer_.PrintBinaryExpressionEnd();
}

void SharpCodeGenerator::operator()(const ast::UnaryExpression &expr) {
  printer_.PrintUnaryExpressionType(expr);
  (*this)(*expr.expression);
}

void SharpCodeGenerator::operator()(const ast::ConstructedValue &value) {
  printer_.PrintConstructedValueBegin(value);
  for (size_t ind = 0; ind < value.fields.size(); ++ind) {
    (*this)(*value.fields[ind].second);
    if (ind != value.fields.size() - 1) {
      *output_ << ", ";
    }
  }
  printer_.PrintConstructedValueEnd();
}

void SharpCodeGenerator::operator()(const ast::Value &value) {
  if (std::holds_alternative<ast::ConstructedValue>(value)) {
    const auto &constructed_value = std::get<ast::ConstructedValue>(value);
    (*this)(constructed_value);
  } else {
    printer_.PrintValue(value);
  }
}

void SharpCodeGenerator::operator()(const ast::VarAccess &var_access) {
  printer_.PrintVarAccess(var_access);
}

void SharpCodeGenerator::operator()(const ast::Star &star) {}

// Helper function. Maybe need to be moved to other file like "utils"

void SharpCodeGenerator::PrintTypedVariables(
    const std::vector<ast::TypedVariable> &variables,
    std::string &&delimeter,
    bool with_types,
    bool add_last_delimeter,
    bool as_dependency,
    bool need_access) {
  bool first = true;
  for (const auto &var : variables) {
    if (first) {
      first = false;
    } else {
      *output_ << delimeter;
    }
    if (with_types) {
      (*this)(var.type_expression, as_dependency, need_access);
      *output_ << " ";
    }
    *output_ << var.name;
  }
  if (add_last_delimeter && !first) {
    *output_ << delimeter;
  }
}

void SharpCodeGenerator::PrintCheck(
  const std::unordered_map<InternedString, std::vector<std::shared_ptr<const ast::Expression>>> &checker_members,
  const std::vector<ast::TypedVariable> &checker_input) {
  *output_ << "\n\tpublic bool Check(";
  PrintTypedVariables(checker_input, ", ", true, false, false, false);
  *output_ << ") {\n";

  // invariant check body
  *output_ << "\t\treturn true";
  for (const auto &[name, expressions] : checker_members) {
    *output_ << " && ";
    *output_ << name << ".Check(";
    for (size_t ind = 0; ind < expressions.size(); ++ind) {
      (*this)(*expressions[ind], false, false);
      if (ind != expressions.size() - 1) {
        *output_ << ", ";
      }
    }
    *output_ << ")";
  }
  *output_ << ";\n\t}\n";
}

bool SharpCodeGenerator::CheckForTriggers(
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
      const auto &constructed_value_name = constructed_value.constructor_identifier.name;
      if (std::holds_alternative<ast::Enum>(
          tree_->types.at(tree_->constructor_to_type.at(constructed_value_name)))) {
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

bool SharpCodeGenerator::IsSimpleType(const InternedString &interned_string) {
  return interned_string == InternedString("Int") ||
         interned_string == InternedString("Unsigned") ||
         interned_string == InternedString("String") ||
         interned_string == InternedString("Float") ||
         interned_string == InternedString("Bool");
}

} // namespace dbuf::gen