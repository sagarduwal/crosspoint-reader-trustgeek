#pragma once

#include "AppCatalogEntry.h"

#include <cstdint>
#include <vector>

// Parses Discover catalog JSON (remote manifest or builtin fallback).
class AppStoreManifestJson {
 public:
  static constexpr uint8_t kFileVersion = 1;

  static bool parse(const char* json, std::vector<AppCatalogEntry>& outEntries, uint8_t& outVersion);
};
