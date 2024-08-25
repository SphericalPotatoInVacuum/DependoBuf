#pragma once

#include "core/codegen/kotlin_target/kotlin_error.h"

#include <iostream>
#include <memory>
#include <ostream>
#include <string_view>

namespace dbuf::gen::kotlin {

enum ControlSymbol {
  NewLine  = 0,
  Separate = 1,
};

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

  template <typename T>
  void Print(const T &message) {
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
    if (need_indent_) {
      for (unsigned int i = 0; i < indent_count_ * kIndentLength; ++i) {
        *output_ << " ";
      }
      need_indent_ = false;
    }
    printable.Print(*this);
  }

  template <>
  void Print(const ControlSymbol &message) {
    if (message == ControlSymbol::NewLine) {
      NewLine();
      return;
    }
    if (message == ControlSymbol::Separate) {
      throw KotlinError("how did u pass here?!");
    }
    throw KotlinError("printer don't support that ControlSymbol");
  }

  template <typename T>
  Printer &operator<<(const T &other) {
    Print(other);
    return *this;
  }

  static const std::string_view kPackageName;

private:
  static const std::string_view kDontChangeMessage;
  static const unsigned int kIndentLength;

  std::shared_ptr<std::ostream> output_;

  unsigned int indent_count_ = 0;
  bool need_indent_          = false;
};

class Scope {
public:
  explicit Scope(Printer &printer);
  void Close();
  virtual ~Scope() = default;

protected:
  virtual void Start() = 0;
  virtual void End()   = 0;

  Printer &printer_;

private:
  bool ended_;
};

template <typename SeparatorType>
class SeparatablePrinter {
public:
  SeparatablePrinter(Printer &printer, SeparatorType separator)
      : printer_(printer)
      , separator_(std::move(separator)) {}

  template <typename T>
  SeparatablePrinter &operator<<(const T &other) {
    if (need_separator_) {
      printer_ << separator_;
      need_separator_ = false;
    }
    printer_ << other;
    return *this;
  }

  template <>
  SeparatablePrinter &operator<<(const ControlSymbol &other) {
    if (other == ControlSymbol::Separate) {
      need_separator_ = true;
      return *this;
    }
    printer_ << other;
    return *this;
  }

  [[nodiscard]] bool NeedSeparator() const {
    return need_separator_;
  }

private:
  Printer &printer_;
  SeparatorType separator_;
  bool need_separator_ = false;
};

} // namespace dbuf::gen::kotlin
