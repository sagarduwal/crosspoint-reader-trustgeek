#include "AppVersion.h"

#include <cctype>

namespace {

bool parseUnsigned(std::string_view text, size_t& pos, int& out) {
  if (pos >= text.size() || !std::isdigit(static_cast<unsigned char>(text[pos]))) {
    return false;
  }
  long value = 0;
  while (pos < text.size() && std::isdigit(static_cast<unsigned char>(text[pos]))) {
    value = value * 10 + (text[pos] - '0');
    if (value > 999999) {
      return false;
    }
    ++pos;
  }
  out = static_cast<int>(value);
  return true;
}

}  // namespace

std::optional<AppVersion> AppVersion::parse(std::string_view text) {
  if (text.empty()) {
    return std::nullopt;
  }

  size_t pos = 0;
  if (text[0] == 'v' || text[0] == 'V') {
    ++pos;
  }

  int major = 0;
  int minor = 0;
  int patch = 0;
  if (!parseUnsigned(text, pos, major)) {
    return std::nullopt;
  }
  if (pos >= text.size() || text[pos] != '.') {
    return std::nullopt;
  }
  ++pos;
  if (!parseUnsigned(text, pos, minor)) {
    return std::nullopt;
  }
  if (pos >= text.size() || text[pos] != '.') {
    return std::nullopt;
  }
  ++pos;
  if (!parseUnsigned(text, pos, patch)) {
    return std::nullopt;
  }

  std::string preRelease;
  if (pos < text.size() && text[pos] == '-') {
    const size_t preStart = pos;
    ++pos;
    while (pos < text.size() && text[pos] != '+') {
      ++pos;
    }
    preRelease = std::string(text.substr(preStart + 1, pos - preStart - 1));
  }

  if (pos < text.size() && text[pos] == '+') {
    ++pos;
    while (pos < text.size()) {
      ++pos;
    }
  }

  if (pos != text.size()) {
    return std::nullopt;
  }

  return AppVersion(major, minor, patch, preRelease, std::string(text));
}

AppVersion::AppVersion(int major, int minor, int patch, std::string preRelease, std::string normalized)
    : major_(major),
      minor_(minor),
      patch_(patch),
      preRelease_(std::move(preRelease)),
      normalized_(std::move(normalized)) {}

bool AppVersion::isNewerThan(const AppVersion& other) const {
  if (major_ != other.major_) {
    return major_ > other.major_;
  }
  if (minor_ != other.minor_) {
    return minor_ > other.minor_;
  }
  if (patch_ != other.patch_) {
    return patch_ > other.patch_;
  }

  if (preRelease_.empty() && !other.preRelease_.empty()) {
    return true;
  }
  if (!preRelease_.empty() && other.preRelease_.empty()) {
    return false;
  }
  if (preRelease_ != other.preRelease_) {
    return preRelease_ > other.preRelease_;
  }
  return false;
}
