#include "AppStoreManifestJson.h"

#include <ArduinoJson.h>

bool AppStoreManifestJson::parse(const char* json, std::vector<AppCatalogEntry>& outEntries, uint8_t& outVersion) {
  outEntries.clear();
  outVersion = 0;

  if (json == nullptr || json[0] == '\0') {
    return false;
  }

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
    AppCatalogEntry entry;
    entry.id = app["id"] | "";
    entry.name = app["name"] | "";
    entry.version = app["version"] | "";
    entry.description = app["description"] | "";
    entry.bundleUrl = app["bundle_url"] | "";
    entry.sha256 = app["sha256"] | "";
    entry.minApiVersion = app["min_api_version"] | "";
    entry.minFirmware = app["min_firmware"] | "";
    if (entry.id.empty() || entry.name.empty() || entry.version.empty()) {
      continue;
    }
    outEntries.push_back(std::move(entry));
  }
  return true;
}
