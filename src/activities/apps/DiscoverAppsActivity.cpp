#include "DiscoverAppsActivity.h"

#include <AppPathSanitizer.h>
#include <AppRegistry.h>
#include <AppStorePaths.h>
#include <AppStoreManifest.h>
#include <GfxRenderer.h>
#include <HalStorage.h>
#include <I18n.h>
#include <Logging.h>
#include <WiFi.h>
#include <ZipFile.h>

#include <cstdio>
#include <cstdlib>

#include "ApplicationsMenuActivity.h"
#include "MappedInputManager.h"
#include "activities/network/WifiSelectionActivity.h"
#include "components/UITheme.h"
#include "fontIds.h"
#include "network/HttpDownloader.h"

namespace {

bool writeBufferToFile(const char* path, const uint8_t* data, size_t size) {
  HalFile outFile;
  if (!Storage.openFileForWrite("APPS", path, outFile)) {
    return false;
  }
  const size_t written = outFile.write(data, size);
  outFile.close();
  return written == size;
}

}  // namespace

DiscoverAppsActivity::DiscoverAppsActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
    : Activity("DiscoverApps", renderer, mappedInput) {}

void DiscoverAppsActivity::loadCatalog() {
  const bool loaded = APP_STORE_CATALOG.load();
  entries_ = APP_STORE_CATALOG.getEntries();
  LOG_DBG("APPS", "Discover catalog load=%s entries=%u", loaded ? "ok" : "fail",
          static_cast<unsigned>(entries_.size()));
  for (const auto& entry : entries_) {
    LOG_DBG("APPS", "Discover app id=%s name=%s version=%s", entry.id.c_str(), entry.name.c_str(), entry.version.c_str());
  }
  APP_REGISTRY.loadFromFile();
  installedEntries_ = APP_REGISTRY.getEntries();
  selectedIndex = 0;
  requestUpdate();
}

void DiscoverAppsActivity::onWifiSelectionComplete(const bool success) {
  if (!success) {
    LOG_ERR("APPS", "Discover WiFi connection failed, returning to Applications");
    activityManager.replaceActivity(std::make_unique<ApplicationsMenuActivity>(renderer, mappedInput));
    return;
  }

  LOG_DBG("APPS", "Discover WiFi connected, loading catalog");
  loadCatalog();
}

void DiscoverAppsActivity::onEnter() {
  Activity::onEnter();
  LOG_DBG("APPS", "Discover activity entered");
  if (hasAttemptedLoad_) {
    // Returning from a sub-activity should not reopen WiFi selection.
    return;
  }
  hasAttemptedLoad_ = true;

  if (WiFi.status() == WL_CONNECTED) {
    LOG_DBG("APPS", "Discover WiFi already connected");
    loadCatalog();
    return;
  }

  LOG_DBG("APPS", "Discover launching WifiSelectionActivity...");
  startActivityForResult(std::make_unique<WifiSelectionActivity>(renderer, mappedInput),
                         [this](const ActivityResult& result) { onWifiSelectionComplete(!result.isCancelled); });
}

void DiscoverAppsActivity::loop() {
  if (mappedInput.wasPressed(MappedInputManager::Button::Back)) {
    activityManager.replaceActivity(std::make_unique<ApplicationsMenuActivity>(renderer, mappedInput));
    return;
  }

  if (entries_.empty()) {
    return;
  }

  if (mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
    if (installSelectedEntry()) {
      APP_REGISTRY.loadFromFile();
      installedEntries_ = APP_REGISTRY.getEntries();
      requestUpdate();
    }
    return;
  }

  const int itemCount = static_cast<int>(entries_.size());
  buttonNavigator.onNextRelease([this, itemCount] {
    selectedIndex = ButtonNavigator::nextIndex(selectedIndex, itemCount);
    requestUpdate();
  });

  buttonNavigator.onPreviousRelease([this, itemCount] {
    selectedIndex = ButtonNavigator::previousIndex(selectedIndex, itemCount);
    requestUpdate();
  });
}

void DiscoverAppsActivity::render(RenderLock&&) {
  renderer.clearScreen();

  const auto& metrics = UITheme::getInstance().getMetrics();
  const auto pageWidth = renderer.getScreenWidth();
  const auto pageHeight = renderer.getScreenHeight();

  GUI.drawHeader(renderer, Rect{0, metrics.topPadding, pageWidth, metrics.headerHeight}, tr(STR_DISCOVER_APPS));

  const int contentTop = metrics.topPadding + metrics.headerHeight + metrics.verticalSpacing;
  const int contentHeight = pageHeight - contentTop - metrics.buttonHintsHeight - metrics.verticalSpacing * 2;
  const int itemCount = static_cast<int>(entries_.size());

  if (itemCount == 0) {
    renderer.drawCenteredText(UI_10_FONT_ID, pageHeight / 2, tr(STR_NO_DISCOVER_APPS));
  } else {
    GUI.drawList(renderer, Rect{0, contentTop, pageWidth, contentHeight}, itemCount, selectedIndex,
                 [this](int index) { return entries_[static_cast<size_t>(index)].name; },
                 [this](int index) {
                   const auto& entry = entries_[static_cast<size_t>(index)];
                   return isInstalled(entry.id) ? std::string(tr(STR_INSTALLED)) : entry.version;
                 });
  }

  const auto labels = mappedInput.mapLabels(tr(STR_BACK), tr(STR_SELECT), tr(STR_DIR_UP), tr(STR_DIR_DOWN));
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);

  renderer.displayBuffer();
}

bool DiscoverAppsActivity::isInstalled(const std::string& appId) const {
  for (const auto& entry : installedEntries_) {
    if (entry.id == appId) {
      return true;
    }
  }
  return false;
}

bool DiscoverAppsActivity::installSelectedEntry() {
  if (selectedIndex < 0 || selectedIndex >= static_cast<int>(entries_.size())) {
    return false;
  }

  const auto& entry = entries_[static_cast<size_t>(selectedIndex)];
  if (!AppPathSanitizer::isValidAppId(entry.id)) {
    LOG_ERR("APPS", "Install rejected: invalid app id %s", entry.id.c_str());
    return false;
  }
  if (entry.bundleUrl.empty()) {
    LOG_ERR("APPS", "Install rejected: app %s has empty bundle URL", entry.id.c_str());
    return false;
  }

  Storage.ensureDirectoryExists(AppStorePaths::kTmpDir);
  Storage.ensureDirectoryExists(AppStorePaths::kAppsRoot);

  char tmpZipPath[96];
  snprintf(tmpZipPath, sizeof(tmpZipPath), "%s/%s.cpapp", AppStorePaths::kTmpDir, entry.id.c_str());
  char appDirPath[96];
  snprintf(appDirPath, sizeof(appDirPath), "%s/%s", AppStorePaths::kAppsRoot, entry.id.c_str());
  char mainLuaPath[112];
  snprintf(mainLuaPath, sizeof(mainLuaPath), "%s/main.lua", appDirPath);
  char manifestPath[112];
  snprintf(manifestPath, sizeof(manifestPath), "%s/manifest.json", appDirPath);

  LOG_DBG("APPS", "Installing app id=%s from %s", entry.id.c_str(), entry.bundleUrl.c_str());
  const auto downloadRes = HttpDownloader::downloadToFile(entry.bundleUrl, tmpZipPath, nullptr);
  if (downloadRes != HttpDownloader::OK) {
    LOG_ERR("APPS", "Install failed: download error %d", static_cast<int>(downloadRes));
    Storage.remove(tmpZipPath);
    return false;
  }

  // #region agent log
  LOG_DBG("APPS", "Install zip expected path=%s", tmpZipPath);
  // #endregion
  ZipFile zip(tmpZipPath);
  size_t mainLuaSize = 0;
  uint8_t* mainLua = zip.readFileToMemory("main.lua", &mainLuaSize, false);
  size_t manifestSize = 0;
  uint8_t* manifest = zip.readFileToMemory("manifest.json", &manifestSize, false);

  if (!mainLua || !manifest) {
    LOG_ERR("APPS", "Install failed: bundle missing required files");
    if (mainLua) free(mainLua);
    if (manifest) free(manifest);
    Storage.remove(tmpZipPath);
    return false;
  }

  Storage.ensureDirectoryExists(appDirPath);
  bool writeOk = writeBufferToFile(mainLuaPath, mainLua, mainLuaSize) && writeBufferToFile(manifestPath, manifest, manifestSize);
  free(mainLua);
  free(manifest);
  Storage.remove(tmpZipPath);

  if (!writeOk) {
    LOG_ERR("APPS", "Install failed: could not write app files for %s", entry.id.c_str());
    return false;
  }

  AppRegistryEntry registryEntry;
  registryEntry.id = entry.id;
  registryEntry.name = entry.name;
  registryEntry.version = entry.version;
  registryEntry.installedAt = "";
  if (!APP_REGISTRY.upsertEntry(registryEntry)) {
    LOG_ERR("APPS", "Install failed: could not update registry for %s", entry.id.c_str());
    return false;
  }

  LOG_INF("APPS", "Installed app id=%s version=%s", entry.id.c_str(), entry.version.c_str());
  return true;
}
