#pragma once

#include <memory>
#include <ostream>
#include <string_view>

namespace dbuf::gen::kotlin {

class Printer;

class PrintableObject {
public:
  virtual void Print(Printer &printer) const = 0;
  virtual ~PrintableObject()                 = default;
};

class Printer {
public:
  explicit Printer(std::shared_ptr<std::ostream> output);

  void AddIndent();
  void RemoveIndent();
  void NewLine();

  bool PrintedSomething() const;
  void StartPrintedCheck();

  template <typename T>
  void Print(const T &message) {
    printed_ = true;
    if (need_indent_) {
      for (unsigned int i = 0; i < indent_count_ * kIndentLength; ++i) {
        *output_ << " ";
      }
      need_indent_ = false;
    }
    *output_ << message;
  }

  template <typename T>
    requires std::is_base_of_v<PrintableObject, T>
  void Print(const T &printable) {
    printed_ = true;
    if (need_indent_) {
      for (unsigned int i = 0; i < indent_count_ * kIndentLength; ++i) {
        *output_ << " ";
      }
      need_indent_ = false;
    }
    printable.Print(*this);
  }

  template <typename T>
  Printer &operator<<(const T &other) {
    Print(other);
    return *this;
  }

private:
  static const std::string_view kDontChangeMessage;
  static const std::string_view kPackageName;
  static const unsigned int kIndentLength;

  std::shared_ptr<std::ostream> output_;

  unsigned int indent_count_ = 0;
  bool need_indent_          = false;
  bool printed_              = false;
};

} // namespace dbuf::gen::kotlin
