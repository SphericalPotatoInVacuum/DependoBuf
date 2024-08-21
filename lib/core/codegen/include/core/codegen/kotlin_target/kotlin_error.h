#pragma once

#include <exception>
#include <string>

namespace dbuf::gen::kotlin {

struct KotlinError : std::exception {
  KotlinError(std::string message);
  const char *what() const noexcept;

private:
  std::string message_;
};

} // namespace dbuf::gen::kotlin
