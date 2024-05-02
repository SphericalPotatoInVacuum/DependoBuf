#include <cstdint>
#include <cstddef>
#include <vector>
#include <fstream>

#include "core/ast/ast.h"
#include "core/checker/checker.h"
#include "core/codegen/generation.h"
#include "core/parser/parse_helper.h"


extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    std::ofstream dbuf_file("test.dbuf", std::ios_base::trunc | std::ios_base::out | std::ios_base::binary);
    dbuf_file << reinterpret_cast<const char*>(data);
    
    dbuf::gen::ListGenerators generators;
    std::vector<std::string> formats{"cpp"};
    try {
        generators.Fill(formats, ".", "test");
    } catch (std::string &err) {
        return 0;
    }

    std::ifstream in_file("test.dbuf");

    dbuf::ast::AST ast;
    dbuf::parser::ParseHelper parse_helper(in_file, std::cerr, &ast);
    try {
        parse_helper.Parse();
    } catch (...) {
        return 0;
    }

    if (dbuf::checker::Checker::CheckAll(ast) != EXIT_SUCCESS) {
        return 0;
    }

    generators.Process(&ast);

    std::system("clang++-16 -std=c++20 test.h 2>>out.txt");
    std::ifstream out("out.txt");
    if (!out.eof()) {
        assert(false);
    }
    return 0;
}