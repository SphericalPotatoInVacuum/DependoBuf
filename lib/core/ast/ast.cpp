#include "core/ast/ast.h"

#include "core/interning/interned_string.h"

#include <cassert>
#include <sstream>
#include <string>
#include <variant>

namespace dbuf::ast::debug {

namespace {

class Defer {
public:
  explicit Defer(const std::function<void()> &other)
      : callback_(other) {}

  ~Defer() {
    callback_();
  }

private:
  std::function<void()> callback_;
};

class AstPrinter {
public:
  void Print(const AST &ast) {
    auto defer = MakeNewScope();
    result_ << current_indent_ << "AST\n";
    result_ << current_indent_ << "VisitOrder\n";
    Print(ast.visit_order);
    result_ << current_indent_ << "Types\n";
    Print(ast.types);
  }

  std::string GetResult() && {
    return result_.str();
  }

private:
  void Print(const std::unordered_map<InternedString, std::variant<Message, Enum>> &types) {
    auto defer = MakeNewScope();
    for (const auto &[name, type] : types) {
      result_ << current_indent_ << "name: " << name << "\n";
      if (std::holds_alternative<Message>(type)) {
        result_ << current_indent_ << "Message\n";
        Print(std::get<Message>(type));
      } else if (std::holds_alternative<Enum>(type)) {
        result_ << current_indent_ << "Enum\n";
        Print(std::get<Enum>(type));
      } else {
        assert(false);
      }
    }
  }

  void Print(const Message &message) {
    auto defer = MakeNewScope();
    result_ << current_indent_ << "identifier: " << message.identifier.name << "\n";
    result_ << current_indent_ << "Field\n";
    Print(message.fields);
    result_ << current_indent_ << "Type dependencies\n";
    Print(message.type_dependencies);
  }

  void Print(const InternedString &string) {
    auto defer = MakeNewScope();
    result_ << current_indent_ << string << "\n";
  }

  void Print(const TypedVariable &variable) {
    auto defer = MakeNewScope();
    result_ << current_indent_ << variable << "\n";
  }

  void Print(const Enum &value) {
    auto defer = MakeNewScope();
    result_ << current_indent_ << "name: " << value.identifier.name << "\n";
    result_ << current_indent_ << "Type dependencies\n";
    Print(value.type_dependencies);
    result_ << current_indent_ << "Pattern mapping\n";
    Print(value.pattern_mapping);
  }

  void Print(const Enum::Rule &rule) {
    auto defer = MakeNewScope();
    result_ << current_indent_ << "Inputs\n";
    Print(rule.inputs);
    result_ << current_indent_ << "Outputs\n";
    Print(rule.outputs);
  }

  void Print(const Enum::Rule::InputPattern &input) {
    auto defer = MakeNewScope();
    if (std::holds_alternative<Value>(input)) {
      result_ << current_indent_ << "value: " << std::get<Value>(input) << "\n";
    } else if (std::holds_alternative<Star>(input)) {
      result_ << current_indent_ << "STAR\n";
    } else {
      assert(false);
    }
  }

  void Print(const Constructor &constr) {
    auto defer = MakeNewScope();
    result_ << current_indent_ << "name: " << constr.identifier.name << "\n";
    result_ << current_indent_ << "Fields\n";
    Print(constr.fields);
  }

  template <typename T>
  void Print(const std::vector<T> &array) {
    for (size_t i = 0; i < array.size(); ++i) {
      Print(array[i]);
    }
  }

  Defer MakeNewScope() {
    current_indent_ += kOneIndent;
    return Defer([this]() {
      assert(current_indent_.size() >= kOneIndent.size());
      current_indent_.resize(current_indent_.size() - kOneIndent.size());
    });
  }

  static constexpr std::string_view kOneIndent = "  ";
  std::string current_indent_;
  std::stringstream result_;
};

} // namespace

std::string ToString(const AST &ast) {
  AstPrinter printer;
  printer.Print(ast);
  return std::move(printer).GetResult();
}

} // namespace dbuf::ast::debug
