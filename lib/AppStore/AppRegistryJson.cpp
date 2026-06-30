#include "AppRegistryJson.h"

#include <ArduinoJson.h>

bool AppRegistryJson::parse(const char* json, std::vector<AppRegistryEntry>& outEntries, uint8_t& outVersion) {
  outEntries.clear();
  outVersion = 0;

  JsonDocument doc;
  const DeserializationError error = deserializeJson(doc, json);
  if (error) {
    return false;
  }

  outVersion = doc["version"] | static_cast<uint8_t>(0);
  JsonArrayConst apps = doc["apps"];
  if (apps.isNull()) {
    return true;
  }

  for (JsonObjectConst app : apps) {
    AppRegistryEntry entry;
    entry.id = app["id"] | "";
    entry.name = app["name"] | "";
    entry.version = app["version"] | "";
    entry.installedAt = app["installed_at"] | "";
    if (entry.id.empty() || entry.name.empty() || entry.version.empty()) {
      continue;
    }
    outEntries.push_back(std::move(entry));
  }
  return true;
}

std::string AppRegistryJson::serialize(const std::vector<AppRegistryEntry>& entries) {
  JsonDocument doc;
  doc["version"] = kFileVersion;
  JsonArray apps = doc["apps"].to<JsonArray>();
  for (const AppRegistryEntry& entry : entries) {
    JsonObject obj = apps.add<JsonObject>();
    obj["id"] = entry.id;
    obj["name"] = entry.name;
    obj["version"] = entry.version;
    if (!entry.installedAt.empty()) {
      obj["installed_at"] = entry.installedAt;
    }
  }

  std::string output;
  serializeJson(doc, output);
  return output;
}
