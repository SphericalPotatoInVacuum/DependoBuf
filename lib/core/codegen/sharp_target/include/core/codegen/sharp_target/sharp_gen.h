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

    void operator()(const ast::TypedVariable &variable);

    void operator()(const ast::Expression &expr);

    void operator()(const ast::TypeExpression &expr);

    void operator()(const ast::BinaryExpression &expr);

    void operator()(const ast::UnaryExpression &expr);

    void operator()(const ast::ConstructedValue &value);

    void operator()(const ast::Value &value);

    void operator()(const ast::VarAccess &var_access);

    static bool IsSimpleType(const InternedString &interned_string);

    const ast::AST *tree_;

    SharpPrinter printer_;
};

} // namespace dbuf::gen
