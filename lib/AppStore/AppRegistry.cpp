#include "AppRegistry.h"

#include <HalStorage.h>
#include <Logging.h>

#include <algorithm>

#include "AppStorePaths.h"

AppRegistry AppRegistry::instance_;

AppRegistry& AppRegistry::getInstance() { return instance_; }

bool AppRegistry::loadFromFile() {
  entries_.clear();

  if (!Storage.exists(AppStorePaths::kRegistryFile)) {
    return true;
  }

  const String json = Storage.readFile(AppStorePaths::kRegistryFile);
  if (json.isEmpty()) {
    LOG_ERR("APPS", "Failed to read registry");
    return false;
  }

  uint8_t version = 0;
  if (!AppRegistryJson::parse(json.c_str(), entries_, version)) {
    LOG_ERR("APPS", "Failed to parse registry.json");
    return false;
  }
  return true;
}

bool AppRegistry::saveToFile() const {
  Storage.ensureDirectoryExists(AppStorePaths::kAppsRoot);

  const std::string json = AppRegistryJson::serialize(entries_);
  if (!Storage.writeFile(AppStorePaths::kRegistryTmpFile, String(json.c_str()))) {
    LOG_ERR("APPS", "Failed to write registry temp file");
    return false;
  }

  if (Storage.exists(AppStorePaths::kRegistryFile)) {
    Storage.remove(AppStorePaths::kRegistryFile);
  }

  if (!Storage.rename(AppStorePaths::kRegistryTmpFile, AppStorePaths::kRegistryFile)) {
    LOG_ERR("APPS", "Failed to rename registry into place");
    Storage.remove(AppStorePaths::kRegistryTmpFile);
    return false;
  }
  return true;
}

bool AppRegistry::upsertEntry(const AppRegistryEntry& entry) {
  for (AppRegistryEntry& existing : entries_) {
    if (existing.id == entry.id) {
      existing = entry;
      return saveToFile();
    }
  }
  entries_.push_back(entry);
  return saveToFile();
}

bool AppRegistry::removeEntry(const std::string& appId) {
  const auto it = std::remove_if(entries_.begin(), entries_.end(),
                                 [&](const AppRegistryEntry& e) { return e.id == appId; });
  if (it == entries_.end()) {
    return true;
  }
  entries_.erase(it, entries_.end());
  return saveToFile();
}
