#include "core/codegen/sharp_target/sharp_print.h"

namespace dbuf::gen {

constexpr unsigned int SharpTypes::HashString(const std::string &str) {
    unsigned int hash = 5381;
    int c;

    auto str_iter = str.begin();
    while ((c = *str_iter++) != 0) {
        hash = ((hash << 5) + hash) + c;
    }

    return hash;
}

constexpr std::string SharpTypes::ConstructSharpType(const std::string &dbuf_type) {
    switch (HashString(dbuf_type)) {
        case HashString("Bool"):
            return kTypes[0];
            break;
        case HashString("Float"):
            return kTypes[1];
            break;
        case HashString("Int"):
            return kTypes[2];
            break;
        case HashString("String"):
            return kTypes[3];
            break;
        case HashString("Unsigned"):
            return kTypes[4];
            break;
    }
    std::cerr << "Unsupported sharp type: " << dbuf_type << std::endl;
    return "";
}

SharpPrinter::SharpPrinter(std::shared_ptr<std::ofstream> output) {
    out_ = std::move(output);
}

void SharpPrinter::PrintVariables(
        const std::vector<ast::TypedVariable> &variables,
        std::string &&delimeter,
        bool add_last_delimeter,
        bool is_public,
        bool need_getter_and_setter) {
    bool first = true;
    for (const auto &var : variables) {
        if (first) {
            first = false;
        } else {
            *out_ << delimeter;
        }
        const std::string &access = is_public ? "public" : "private";
        const std::string &type = GetVariableType(var);
        const std::string &name = GetVariableName(var);
        *out_ << access << " " << type_constructor_.ConstructSharpType(type) << " " << name;
        if (need_getter_and_setter) {
            *out_ << " " << kGetterAndSetter;
        }
    }
    if (add_last_delimeter && !first) {
        *out_ << delimeter;
    }
}

void SharpPrinter::PrintClassBegin(const std::string &name) {
    *out_ << "public class " << name << " {\n";
}

void SharpPrinter::PrintClassEnd() {
    *out_ << "}\n";
}

std::string SharpPrinter::GetVariableName(const ast::TypedVariable &var) {
    return var.name.GetString();
}

std::string SharpPrinter::GetVariableType(const ast::TypedVariable &var) {
    return var.type_expression.identifier.name.GetString();
}

} // namespace dbuf::gen