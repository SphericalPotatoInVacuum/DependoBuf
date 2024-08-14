#include "core/codegen/dart_gen.h"
#include <unordered_map>
#include <unordered_set>

const std::unordered_map<std::string, std::string> simple_types {
    {"Int", "int"},
    {"Float", "float"},
    {"Double", "double"},
    {"String", "String"},
    {"Bool", "bool"},
    {"Unsigned", "int"}
};

namespace dbuf::gen {
void DartCodeGenerator::Generate(ast::AST* tree) {
    if (tree == nullptr) {
        std::cerr << "Dart codegen did\'t get AST" << std::endl;
    }
    *output_ << "//Auto codegen for Dart" << "\n\n";
    for (auto& node : tree->visit_order) { //обходим аст, дин полимор = visit
        std::visit(*this, tree->types[node]);
    }
}

void DartCodeGenerator::operator()(const ast::Message& ast_message) {
    *output_ << "class " << ast_message.identifier.name << "{\n";
    if (ast_message.fields.empty()) {
        *output_ << "}\n\n";
        return;
    }
    std::string con_args_str; //аргументы конструктора
    std::string con_tab(ast_message.identifier.name.GetString().size() + 3, ' ');
    std::string con_assign_str = con_tab + ": "; //присвоения аргументов

    for (const auto& field : ast_message.fields) { //заполняем поля
        std::string type_name = field.type_expression.identifier.name.GetString();

        con_assign_str += "this." + field.name.GetString() + " = " +
        field.name.GetString() + ",\n" + con_tab + "  ";

        if (simple_types.contains(type_name)) {
            con_args_str += (std::string)simple_types.at(type_name) + ' '
            + field.name.GetString() + ", ";
            
            *output_ << "    " << simple_types.at(type_name) << "? " << field.name << ";\n";
            continue;
        }
        con_args_str += type_name + ' '
        + field.name.GetString() + ", ";

        *output_ << "    " << type_name << "? " << field.name << ";\n";
    }
    con_assign_str[con_assign_str.length() - con_tab.length() - 4] = ';';
    con_args_str[con_args_str.length() - 2] = ')';
    *output_ << '\n';
    *output_ << "    " << ast_message.identifier.name << '(' << con_args_str <<
    '\n' << con_assign_str << '\n';

    *output_ << "    bool check(";

    if (!ast_message.type_dependencies.empty()) {
        std::string check_args;
        for (auto& dependency : ast_message.type_dependencies) {
            std::string type_name = dependency.type_expression.identifier.name.GetString();
            if (simple_types.contains(type_name)) {
                check_args += (std::string)simple_types.at(type_name);
            } else {
                check_args += type_name;
            }
            check_args += ' ' + dependency.name.GetString() + ", ";
        }
        check_args[check_args.length() - 1] = ' ';
        check_args[check_args.length() - 2] = ')';
        *output_ << check_args << "{return true;}";

    } else {
        *output_ << ") {return true;}";
    }

    *output_ << "\n}\n\n";
}

void DartCodeGenerator::operator()(const ast::Enum& ast_enum) {
    *output_ << "class " << ast_enum.identifier.name << "{\n";
    std::string constructors_str;
    std::unordered_set<std::string> field_set;
    for (const auto& pattern : ast_enum.pattern_mapping) {
        for (const auto& construct : pattern.outputs) {
            if (construct.fields.empty()) {
                constructors_str += "    " + ast_enum.identifier.name.GetString() +
                '.' + construct.identifier.name.GetString() + "();\n";
                continue;
            }
            std::string con_args_str; //аргументы конструктора
            std::string con_tab(ast_enum.identifier.name.GetString().size() +
            construct.identifier.name.GetString().size() + 3, ' ');
            std::string con_assign_str = con_tab + ": "; //присвоения аргументов

            for (const auto& field : construct.fields) { //заполняем поля
                std::string type_name = field.type_expression.identifier.name.GetString();

                con_assign_str += "this." + field.name.GetString() + " = " +
                field.name.GetString() + ",\n" + con_tab + "  ";

                if (simple_types.contains(type_name)) {
                    con_args_str += (std::string)simple_types.at(type_name) + ' '
                    + field.name.GetString() + ", ";
                    
                    field_set.insert("    " + simple_types.at(type_name) +
                    "? " + field.name.GetString() + ";\n");
                    continue;
                }
                con_args_str += type_name + ' '
                + field.name.GetString() + ", ";

                field_set.insert("    " + type_name +
                "? " + field.name.GetString() + ";\n");
            }
            con_assign_str[con_assign_str.length() - con_tab.length() - 4] = ';';
            con_args_str[con_args_str.length() - 2] = ')';
            constructors_str += "    " + ast_enum.identifier.name.GetString() + '.' +
            construct.identifier.name.GetString() + '(' + con_args_str + '\n' +
            con_assign_str + '\n';
        }
    }
    for (auto& c : field_set) {
        *output_ << c;
    }
    *output_ << '\n';
    *output_ << constructors_str;
    *output_ << "    bool check(";
    if (!ast_enum.type_dependencies.empty()) {
        std::vector<std::string> depends;
        std::string check_args;
        for (auto& dependency : ast_enum.type_dependencies) {
            std::string type_name = dependency.type_expression.identifier.name.GetString();
            if (simple_types.contains(type_name)) {
                check_args += (std::string)simple_types.at(type_name);
            } else {
                check_args += type_name;
            }
            check_args += ' ' + dependency.name.GetString() + ", ";
            depends.push_back(dependency.name.GetString());
        }
        check_args[check_args.length() - 1] = ' ';
        check_args[check_args.length() - 2] = ')';
        *output_ << check_args << '{';
        if (ast_enum.pattern_mapping.size() > 1) {
            *output_ << "\n        if (";
            const auto& pattern = ast_enum.pattern_mapping[0];
            for (size_t j = 0; j < depends.size(); ++j) {
                if (pattern.inputs[j].index() == 0) {
                    *output_ << depends[j] << " == " << pattern.inputs[j] << " && ";
                }
            }
            *output_ << "true) {\n            return ";
            for (const auto& construct : pattern.outputs) {
                for (const auto& field : construct.fields) {
                    *output_ << "(this." << field.name << " != null) && ";
                }
            }
            *output_ << "true;\n        } else ";
            for (size_t i = 1; i < ast_enum.pattern_mapping.size() - 1; ++i) {
                *output_ << "if (";
                const auto& pattern = ast_enum.pattern_mapping[i];
                for (size_t j = 0; j < depends.size(); ++j) {
                    if (pattern.inputs[j].index() == 0) {
                        *output_ << depends[j] << " == " << pattern.inputs[j] << " && ";
                    }
                }
                *output_ << "true) {\n            return ";
                for (const auto& construct : pattern.outputs) {
                    for (const auto& field : construct.fields) {
                        *output_ << "(this." << field.name << " != null) && ";
                    }
                }
                *output_ << "true;\n        } else ";
            }
            const auto& pattern_ = ast_enum.pattern_mapping[ast_enum.pattern_mapping.size() - 1];
            *output_ << "{\n            return ";
            for (const auto& construct : pattern_.outputs) {
                for (const auto& field : construct.fields) {
                    *output_ << "(this." << field.name << " != null) && ";
                }
            }
            
            *output_ << "true;\n        }";
            *output_ << "\n    }";
        } else {
            *output_ << "return ";
            const auto& pattern_ = ast_enum.pattern_mapping[0];
            for (const auto& construct : pattern_.outputs) {
                for (const auto& field : construct.fields) {
                    *output_ << "(this." << field.name << " != null) && ";
                }
            }
            
            *output_ << "true;\n    }";
            
            
        }
    } else {
        *output_ << ") {return true;}";
    }
    *output_ << "\n}\n\n";
}
}