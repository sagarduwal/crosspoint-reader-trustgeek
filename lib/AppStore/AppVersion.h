#pragma once

#include <optional>
#include <string>
#include <string_view>

// Semantic version comparison for App Store manifests and registry entries.
// Supports optional pre-release suffix (e.g. 1.2.3-beta) and build metadata (+build).
class AppVersion {
 public:
  static std::optional<AppVersion> parse(std::string_view text);

  // Returns true if this version is strictly newer than other.
  bool isNewerThan(const AppVersion& other) const;

  const std::string& text() const { return normalized_; }

 private:
  AppVersion(int major, int minor, int patch, std::string preRelease, std::string normalized);

  int major_ = 0;
  int minor_ = 0;
  int patch_ = 0;
  std::string preRelease_;
  std::string normalized_;
};
