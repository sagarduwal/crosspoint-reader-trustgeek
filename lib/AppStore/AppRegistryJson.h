#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct AppRegistryEntry {
  std::string id;
  std::string name;
  std::string version;
  std::string installedAt;
};

// Parses and serializes /.crosspoint/apps/registry.json.
// SD I/O is handled by AppRegistry; JSON logic is testable via these helpers.
class AppRegistryJson {
 public:
  static constexpr uint8_t kFileVersion = 1;

  static bool parse(const char* json, std::vector<AppRegistryEntry>& outEntries, uint8_t& outVersion);
  static std::string serialize(const std::vector<AppRegistryEntry>& entries);
};
