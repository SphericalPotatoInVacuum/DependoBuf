#pragma once

#include <exception>
#include <string>

namespace dbuf::gen::kotlin {

struct KotlinError : std::exception {
  explicit KotlinError(std::string message);
  [[nodiscard]] const char *what() const noexcept override;

private:
  std::string message_;
};

} // namespace dbuf::gen::kotlin
