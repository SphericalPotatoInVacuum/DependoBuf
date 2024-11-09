#include "core/codegen/kotlin_target/kotlin_error.h"

namespace dbuf::gen::kotlin {

KotlinError::KotlinError(std::string message)
    : message_("kotlin code generation: " + std::move(message)) {}

const char *KotlinError::what() const noexcept {
  return message_.c_str();
}

} // namespace dbuf::gen::kotlin
