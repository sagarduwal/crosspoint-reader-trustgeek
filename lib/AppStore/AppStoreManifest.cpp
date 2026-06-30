#include "AppStoreManifest.h"

#include <HalStorage.h>
#include <Logging.h>
#include <WiFi.h>

#include "network/HttpDownloader.h"

#include <string>

#include "AppStoreManifestData.h"
#include "AppStorePaths.h"

AppStoreManifest AppStoreManifest::instance_;

AppStoreManifest& AppStoreManifest::getInstance() { return instance_; }

bool AppStoreManifest::tryLoadRemote(std::string& outJson) {
  outJson.clear();

  if (WiFi.status() != WL_CONNECTED) {
    return false;
  }

  const bool ok = HttpDownloader::fetchUrl(AppStoreManifestData::kRemoteUrl, outJson);
  return ok && !outJson.empty();
}

bool AppStoreManifest::tryLoadCache(std::string& outJson) {
  outJson.clear();

  if (!Storage.exists(AppStorePaths::kManifestCacheFile)) {
    return false;
  }

  const String cached = Storage.readFile(AppStorePaths::kManifestCacheFile);
  if (cached.isEmpty()) {
    return false;
  }

  outJson = cached.c_str();
  return true;
}

bool AppStoreManifest::writeCache(const std::string& json) const {
  if (json.empty()) {
    return false;
  }

  Storage.ensureDirectoryExists(AppStorePaths::kCacheDir);

  static constexpr char kCacheTmp[] = "/.crosspoint/apps/_cache/manifest.json.tmp";
  if (!Storage.writeFile(kCacheTmp, String(json.c_str()))) {
    LOG_ERR("APPS", "Failed to write manifest cache temp file");
    return false;
  }

  if (Storage.exists(AppStorePaths::kManifestCacheFile)) {
    Storage.remove(AppStorePaths::kManifestCacheFile);
  }

  if (!Storage.rename(kCacheTmp, AppStorePaths::kManifestCacheFile)) {
    LOG_ERR("APPS", "Failed to rename manifest cache into place");
    Storage.remove(kCacheTmp);
    return false;
  }
  return true;
}

bool AppStoreManifest::load() {
  entries_.clear();
  source_ = AppCatalogSource::None;

  std::string remoteJson;
  const bool remoteFetchOk = tryLoadRemote(remoteJson);

  std::string cacheJson;
  const bool cacheReadOk = tryLoadCache(cacheJson);

  AppCatalogResolveInput input;
  input.remoteJson = remoteFetchOk ? remoteJson.c_str() : nullptr;
  input.remoteFetchOk = remoteFetchOk;
  input.cacheJson = cacheReadOk ? cacheJson.c_str() : nullptr;
  input.cacheReadOk = cacheReadOk;
  input.builtinJson = AppStoreManifestData::kBuiltinJson;

  uint8_t version = 0;
  if (!resolveAndParse(input, entries_, version, source_)) {
    LOG_ERR("APPS", "Failed to load Discover catalog from all sources");
    return false;
  }

  if (source_ == AppCatalogSource::Remote && !writeCache(remoteJson)) {
    LOG_ERR("APPS", "Discover catalog loaded from remote but cache write failed");
  }

  const char* sourceLabel = "unknown";
  switch (source_) {
    case AppCatalogSource::Remote:
      sourceLabel = "remote";
      break;
    case AppCatalogSource::Cache:
      sourceLabel = "cache";
      break;
    case AppCatalogSource::Builtin:
      sourceLabel = "builtin";
      break;
    default:
      break;
  }

  LOG_INF("APPS", "Discover catalog: %u apps from %s (manifest v%u)", static_cast<unsigned>(entries_.size()),
          sourceLabel, version);
  return true;
}
