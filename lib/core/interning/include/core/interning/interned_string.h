/*
This file is part of DependoBuf project.

Copyright (C) 2023 Alexander Bogdanov, Alice Vernigor

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/
#pragma once

#include <cstdint>
#include <functional>
#include <limits>
#include <ostream>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace dbuf {

class InternedString {
public:
  InternedString() = default;

  explicit InternedString(const uint64_t id)
      : id_(id) {}

  explicit InternedString(const std::string &str);
  explicit InternedString(std::string &&str);

  [[nodiscard]] uint64_t GetId() const;
  [[nodiscard]] const std::string &GetString() const;

  bool operator==(const InternedString &other) const;
  bool operator<(const InternedString &other) const;
  friend std::ostream &operator<<(std::ostream &os, const InternedString &str);

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
