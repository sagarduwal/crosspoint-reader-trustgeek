#pragma once

#include <optional>
#include <string>
#include <string_view>

// Validates and normalizes app filesystem paths for sandbox enforcement.
class AppPathSanitizer {
 public:
  struct Result {
    std::string normalized;
  };

  // Reject absolute paths, parent traversal, and empty segments.
  static std::optional<Result> sanitizeRelativePath(std::string_view relativePath);

  // Ensure path stays within /.crosspoint/apps/<appId>/data after joining.
  static bool isPathUnderDataRoot(std::string_view appId, std::string_view relativePath);

  // Validate app id used as directory name (alphanumeric, dash, underscore).
  static bool isValidAppId(std::string_view appId);
};
