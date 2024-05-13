#pragma once

#include <array>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include "core/ast/ast.h"
#include "core/ast/expression.h"
#include "core/interning/interned_string.h"

namespace dbuf::gen {

static const std::string kReadme = "// This file was automatically generated by DependoBuf.\n"
                                   "// Please do not change it.\n";

static const std::string kIncludedLibraries = "using System;\n"
                                              "using System.Collections.Generic;\n";

static const std::string kOpenNamespace = "class dbuf {\n";

static const std::string kCloseNamespace = "}\n";

static const std::string kBaseEnumFields = "\tprivate readonly List<Type> Restrictions;\n"
                                           "\tprivate dynamic _value;\n\n"
                                           "\tpublic dynamic Value {\n"
                                           "\t\tget { return _value; }\n"
                                           "\t\tset {\n"
                                           "\t\t\tforeach (var type in Restrictions) {\n"
                                           "\t\t\t\tif (type.IsInstanceOfType(value)) {\n"
                                           "\t\t\t\t\t_value = value;\n"
                                           "\t\t\t\t\treturn;\n"
                                           "\t\t\t\t}\n"
                                           "\t\t\t}\n"
                                           "\t\t\tthrow new ArgumentException(\"Incorrect argument type\");\n"
                                           "\t\t}\n"
                                           "\t}\n";

static const std::string kBaseEnumConstructor = "\t\tRestrictions = new List<Type>();\n"
                                                "\t\tforeach (var type in AllowedTypes) {\n"
                                                "\t\t\tvar val = (dynamic)Activator.CreateInstance(type, a_);\n"
                                                "\t\t\tif (val.Check()) {\n"
                                                "\t\t\t\tforeach (var restr in val.restrictions) {\n"
                                                "\t\t\t\t\tRestrictions.Add(restr);\n"
                                                "\t\t\t\t}\n"
                                                "\t\t\t}\n"
                                                "\t\t}\n";

static const std::string kBaseEnumCheck = "\tpublic bool Check() {\n"
                                          "\t\tforeach (var type in Restrictions) {\n"
                                          "\t\t\tif (type.IsInstanceOfType(_value)) {\n"
                                          "\t\t\t\tif (type.GetMethod(\"Check\") != null) {\n"
                                          "\t\t\t\t\treturn _value.Check();\n"
                                          "\t\t\t\t} else {\n"
                                          "\t\t\t\t\treturn true;\n"
                                          "\t\t\t\t}\n"
                                          "\t\t\t}\n"
                                          "\t\t}\n"
                                          "\t\treturn false;\n"
                                          "\t}\n";

class SharpTypes {
public:
    static constexpr std::string ConstructSharpType(const std::string &dbuf_type);

private:
    static constexpr unsigned int HashString(const std::string &str);
    
    static constexpr const char* kTypes[] = {"bool", "float", "int", "string", "unsigned"};
};

class SharpPrinter {
public:
    SharpTypes type_constructor_;

    SharpPrinter() = default;

    explicit SharpPrinter(std::shared_ptr<std::ofstream> output);

    void InitFile();

    void CompleteFile();

    void PrintClassBegin(const std::string &name);

    void PrintClassEnd();

    void PrintConstructorBegin(
        const std::string &name,
        const std::vector<std::pair<InternedString, InternedString>> &dependent_variables,
        bool print_vars = true);
    
    void PrintConstructorEnd();

    void PrintBaseEnumFields();

    void PrintBaseEnumConstructor();

    void PrintBaseEnumCheck();
    
    void PrintTypeExpression(
        const ast::TypeExpression &expression,
        bool is_public,
        bool as_dependency = false,
        bool need_access = true);
    
    void PrintBinaryExpressionBegin();

    void PrintBinaryExpressionType(const ast::BinaryExpression &binary_expr);

    void PrintBinaryExpressionEnd();

    void PrintUnaryExpressionType(const ast::UnaryExpression &unary_expr);

    void PrintConstructedValueBegin(const ast::ConstructedValue &constr_value);

    void PrintConstructedValueEnd();
    
    void PrintValue(const ast::Value &value);

    void PrintVarAccess(const ast::VarAccess &var_access);

private:
    static std::string GetVariableName(const ast::TypedVariable &var);

    static std::string GetVariableType(const ast::TypedVariable &var);

    static std::string GetExpressionType(const ast::TypeExpression &expr);

    std::shared_ptr<std::ofstream> out_ = nullptr;
};

} // namespace dbuf::gen
