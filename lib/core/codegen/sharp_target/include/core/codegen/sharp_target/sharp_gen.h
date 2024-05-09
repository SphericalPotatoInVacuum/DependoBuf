#pragma once

#include "core/codegen/generation.h"
#include "core/codegen/sharp_target/sharp_print.h"

namespace dbuf::gen {

class SharpCodeGenerator : public ITargetCodeGenerator {
public:
    explicit SharpCodeGenerator(const std::string &out_file);
    
    void Generate(const ast::AST *tree) override;

private:
    void operator()(const ast::Message &ast_message);

    void operator()(const ast::Enum &ast_enum);

    const ast::AST *tree_;

    SharpPrinter printer_;
};

} // namespace dbuf::gen
