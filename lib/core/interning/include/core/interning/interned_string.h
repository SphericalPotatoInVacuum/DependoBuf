#pragma once

#include <cstdint>
#include <functional>
#include <limits>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace dbuf {

class InternedString {
public:
  InternedString()                                           = default;
  InternedString(const InternedString &other)                = default;
  InternedString(InternedString &&other) noexcept            = default;
  InternedString &operator=(const InternedString &other)     = default;
  InternedString &operator=(InternedString &&other) noexcept = default;

  explicit InternedString(const uint64_t id)
      : id_(id) {}

  explicit InternedString(const std::string &str);
  explicit InternedString(std::string &&str);

  [[nodiscard]] uint64_t GetId() const;
  [[nodiscard]] const std::string &GetString() const;

  bool operator==(const InternedString &other) const;

private:
  static constexpr uint64_t kInvalidId = std::numeric_limits<uint64_t>::max();

  static std::unordered_map<std::string, uint64_t> string_map_;
  static std::unordered_map<uint64_t, std::reference_wrapper<const std::string>> id_map_;

  uint64_t id_ = kInvalidId;
};

} // namespace dbuf

template <>
struct std::hash<dbuf::InternedString> {
  std::size_t operator()(const dbuf::InternedString &k) const {
    return std::hash<uint64_t>()(k.GetId());
  }
};
