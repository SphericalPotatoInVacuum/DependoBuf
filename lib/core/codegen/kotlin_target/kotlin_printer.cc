#include "core/codegen/kotlin_target/kotlin_printer.h"

namespace dbuf::gen::kotlin {

Printer::Printer(std::shared_ptr<std::ostream> output)
    : output_(std::move(output)) {
  *output_ << "package " << kPackageName << "\n\n";
  need_indent_ = true;
}

void Printer::AddIndent() {
  ++indent_count_;
}

void Printer::RemoveIndent() {
  --indent_count_;
}

void Printer::NewLine() {
  *output_ << "\n";
  need_indent_ = true;
}

bool Printer::PrintedSomething() const {
  return printed_;
}
void Printer::StartPrintedCheck() {
  printed_ = false;
}

} // namespace dbuf::gen::kotlin
