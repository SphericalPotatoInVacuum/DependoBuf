#include "core/codegen/dart_gen.h"
#include <map>

const std::map<std::string, std::string> simple_types {
    {"Int", "int"},
    {"Float", "float"},
    {"Double", "double"},
    {"String", "String"},
    {"Bool", "bool"},
    {"Vec", "List"},
    {"Unsigned", "unsigned"}
};

namespace dbuf::gen {
void DartCodeGenerator::Generate(ast::AST* tree) {
    if (tree == nullptr) {
        std::cerr << "Dart codegen did\'t get AST" << std::endl;
    }
}
}