#pragma once

#include "core/codegen/dart_gen.h"
#include "core/ast/ast.h"
#include "core/codegen/generation.h"

namespace dbuf::gen {

class DartCodeGenerator : public ITargetCodeGenerator {
public:
    explicit DartCodeGenerator(const std::string& out_file) : ITargetCodeGenerator(out_file) {}

    void operator()(const ast::Message& ast_message);

    void operator()(const ast::Enum& ast_message);

    void Generate(ast::AST* tree) override;

};
}