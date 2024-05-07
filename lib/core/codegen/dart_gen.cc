#include "core/codegen/dart_gen.h"
#include <unordered_map>

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
    std::string con_args_str = ""; //аргументы конструктора
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

    *output_ << "}\n\n";
}
}