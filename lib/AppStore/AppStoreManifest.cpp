#include "AppStoreManifest.h"

#include <HalStorage.h>
#include <Logging.h>
#include <WiFi.h>

#include "network/HttpDownloader.h"

#include <string>

#include "AppStoreManifestData.h"
#include "AppStorePaths.h"

namespace {

bool isWifiReadyForManifestFetch(wl_status_t wifiStatus, const IPAddress& localIp) {
  if (wifiStatus != WL_CONNECTED) {
    return false;
  }
  return !(localIp[0] == 0 && localIp[1] == 0 && localIp[2] == 0 && localIp[3] == 0);
}

}  // namespace

AppStoreManifest AppStoreManifest::instance_;

AppStoreManifest& AppStoreManifest::getInstance() { return instance_; }

bool AppStoreManifest::tryLoadRemote(std::string& outJson) {
  outJson.clear();

  const wl_status_t wifiStatus = WiFi.status();
  const IPAddress localIp = WiFi.localIP();
  const bool wifiReady = isWifiReadyForManifestFetch(wifiStatus, localIp);
  LOG_DBG("APPS", "Discover preflight wifi_status=%d ip=%u.%u.%u.%u ready=%d", static_cast<int>(wifiStatus),
          static_cast<unsigned>(localIp[0]), static_cast<unsigned>(localIp[1]), static_cast<unsigned>(localIp[2]),
          static_cast<unsigned>(localIp[3]), wifiReady ? 1 : 0);
  if (!wifiReady) {
    LOG_DBG("APPS", "Discover remote fetch skipped (wifi not ready)");
    return false;
  }

  const bool ok = HttpDownloader::fetchUrl(AppStoreManifestData::kRemoteUrl, outJson);
  LOG_DBG("APPS", "Discover remote fetch attempted ok=%d bytes=%u", ok ? 1 : 0, static_cast<unsigned>(outJson.size()));
  if (ok && !outJson.empty()) {
    const size_t previewLen = outJson.size() < 80 ? outJson.size() : static_cast<size_t>(80);
    LOG_DBG("APPS", "Discover remote preview=%.*s", static_cast<int>(previewLen), outJson.c_str());
  }
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
  LOG_DBG("APPS", "Remote fetch=%s", remoteFetchOk ? "ok" : "fail");

  std::string cacheJson;
  const bool cacheReadOk = tryLoadCache(cacheJson);
  LOG_DBG("APPS", "Cache read=%s", cacheReadOk ? "ok" : "fail");

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
