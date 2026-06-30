#include "AppPathSanitizer.h"

#include <cctype>

namespace {

bool hasParentSegment(std::string_view path) {
  size_t pos = 0;
  while (pos < path.size()) {
    while (pos < path.size() && path[pos] == '/') {
      ++pos;
    }
    if (pos >= path.size()) {
      break;
    }
    const size_t start = pos;
    while (pos < path.size() && path[pos] != '/') {
      ++pos;
    }
    const std::string_view segment = path.substr(start, pos - start);
    if (segment == "..") {
      return true;
    }
  }
  return false;
}

}  // namespace

std::optional<AppPathSanitizer::Result> AppPathSanitizer::sanitizeRelativePath(std::string_view relativePath) {
  if (relativePath.empty()) {
    return std::nullopt;
  }
  if (relativePath[0] == '/') {
    return std::nullopt;
  }
  if (hasParentSegment(relativePath)) {
    return std::nullopt;
  }

  std::string normalized(relativePath);
  while (!normalized.empty() && normalized.back() == '/') {
    normalized.pop_back();
  }
  if (normalized.empty()) {
    return std::nullopt;
  }
  return Result{std::move(normalized)};
}

bool AppPathSanitizer::isPathUnderDataRoot(std::string_view appId, std::string_view relativePath) {
  if (!isValidAppId(appId)) {
    return false;
  }
  const auto sanitized = sanitizeRelativePath(relativePath);
  return sanitized.has_value();
}

bool AppPathSanitizer::isValidAppId(std::string_view appId) {
  if (appId.empty() || appId.size() > 64) {
    return false;
  }
  for (const char ch : appId) {
    if (std::isalnum(static_cast<unsigned char>(ch)) || ch == '-' || ch == '_') {
      continue;
    }
    return false;
  }
  return true;
}
