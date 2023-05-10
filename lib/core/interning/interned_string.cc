#include "core/interning/interned_string.h"

#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>

namespace dbuf {

InternedString::InternedString(const std::string &str)
    : InternedString(std::string(str)) {}

InternedString::InternedString(std::string &&str) {
  auto result = string_map_.try_emplace(std::move(str), string_map_.size());
  if (result.second) {
    id_map_.emplace(result.first->second, std::ref(result.first->first));
  }
  id_ = result.first->second;
}

uint64_t InternedString::GetId() const {
  if (id_ == kInvalidId) {
    throw std::runtime_error("InternedString id not initialized");
  }

  return id_;
}

const std::string &InternedString::GetString() const {
  if (id_ == kInvalidId) {
    throw std::runtime_error("InternedString id not initialized");
  }

  auto iter = id_map_.find(id_);
  if (iter != id_map_.end()) {
    return iter->second;
  }
  throw std::runtime_error("InternedString id not found in id_map");
}

bool InternedString::operator==(const InternedString &other) const {
  return id_ == other.id_;
}

bool InternedString::operator<(const InternedString &other) const {
  if (id_ == kInvalidId || other.id_ == kInvalidId) {
    throw std::runtime_error("InternedString id not initialized");
  }
  return GetString() < other.GetString();
}

std::ostream &operator<<(std::ostream &os, const InternedString &str) {
  os << str.GetString();
  return os;
}

std::unordered_map<std::string, uint64_t> InternedString::string_map_;
std::unordered_map<uint64_t, std::reference_wrapper<const std::string>> InternedString::id_map_;

} // namespace dbuf
