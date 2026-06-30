#pragma once

#include "AppRegistryJson.h"

#include <vector>

// Singleton index of installed apps on SD card.
class AppRegistry {
 public:
  static AppRegistry& getInstance();

  AppRegistry(const AppRegistry&) = delete;
  AppRegistry& operator=(const AppRegistry&) = delete;

  bool loadFromFile();
  bool saveToFile() const;

  const std::vector<AppRegistryEntry>& getEntries() const { return entries_; }

  bool upsertEntry(const AppRegistryEntry& entry);
  bool removeEntry(const std::string& appId);

 private:
  AppRegistry() = default;

  std::vector<AppRegistryEntry> entries_;
  static AppRegistry instance_;
};

#define APP_REGISTRY AppRegistry::getInstance()
