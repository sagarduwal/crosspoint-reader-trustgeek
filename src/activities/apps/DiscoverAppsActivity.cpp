#include "DiscoverAppsActivity.h"

#include <AppPathSanitizer.h>
#include <AppRegistry.h>
#include <AppStorePaths.h>
#include <AppStoreManifest.h>
#include <GfxRenderer.h>
#include <HalStorage.h>
#include <I18n.h>
#include <Logging.h>
#include <Memory.h>
#include <WiFi.h>
#include <ZipFile.h>

#include <cstring>

#include <cstdio>
#include <cstdlib>
#include <string_view>

#include "ApplicationsMenuActivity.h"
#include "MappedInputManager.h"
#include "activities/network/WifiSelectionActivity.h"
#include "components/UITheme.h"
#include "fontIds.h"
#include "network/HttpDownloader.h"

namespace {

bool ensureParentDirectoryExists(const char* filePath) {
  const char* lastSlash = strrchr(filePath, '/');
  if (lastSlash == nullptr || lastSlash == filePath) {
    return true;
  }
  char dirPath[128];
  const size_t len = static_cast<size_t>(lastSlash - filePath);
  if (len >= sizeof(dirPath)) {
    return false;
  }
  memcpy(dirPath, filePath, len);
  dirPath[len] = '\0';
  if (Storage.exists(dirPath)) {
    return true;
  }
  return Storage.mkdir(dirPath, true);
}

bool extractZipEntry(ZipFile& zip, const char* zipEntry, const char* destPath) {
  if (!ensureParentDirectoryExists(destPath)) {
    return false;
  }

  HalFile outFile;
  if (!Storage.openFileForWrite("APPS", destPath, outFile)) {
    LOG_ERR("APPS", "Install failed: could not open %s for write", destPath);
    return false;
  }

  const bool ok = zip.readFileToStream(zipEntry, outFile, 1024);
  outFile.close();
  if (!ok) {
    Storage.remove(destPath);
  }
  return ok;
}

static constexpr size_t kMaxBundleFiles = 48;
static constexpr size_t kMaxBundleEntryLen = 96;

struct BundleFileEntry {
  char zipPath[kMaxBundleEntryLen];
  char destPath[128];
};

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

  LOG_DBG("APPS", "Installing app id=%s from %s", entry.id.c_str(), entry.bundleUrl.c_str());
  const auto downloadRes = HttpDownloader::downloadToFile(entry.bundleUrl, tmpZipPath, nullptr);
  if (downloadRes != HttpDownloader::OK) {
    LOG_ERR("APPS", "Install failed: download error %d", static_cast<int>(downloadRes));
    Storage.remove(tmpZipPath);
    return false;
  }

  if (Storage.exists(appDirPath)) {
    if (!Storage.removeDir(appDirPath)) {
      LOG_ERR("APPS", "Install failed: could not remove existing app dir %s", appDirPath);
      Storage.remove(tmpZipPath);
      return false;
    }
  }
  Storage.ensureDirectoryExists(appDirPath);

  ZipFile zip(tmpZipPath);
  bool extractOk = true;
  bool hasMainLua = false;
  bool hasManifest = false;
  auto bundleFiles = makeUniqueNoThrow<BundleFileEntry[]>(kMaxBundleFiles);
  if (!bundleFiles) {
    LOG_ERR("APPS", "Install failed: OOM bundle file table");
    Storage.remove(tmpZipPath);
    return false;
  }
  size_t bundleFileCount = 0;

  // Collect paths first — readFileToMemory inside enumerate corrupts the central-dir cursor.
  zip.enumerateFilePaths([&](const std::string_view entryPath) {
    if (!extractOk) {
      return;
    }

    if (entryPath.empty() || entryPath.back() == '/') {
      return;
    }

    const auto sanitized = AppPathSanitizer::sanitizeRelativePath(entryPath);
    if (!sanitized.has_value()) {
      LOG_ERR("APPS", "Install rejected: unsafe bundle path %.*s", static_cast<int>(entryPath.size()), entryPath.data());
      extractOk = false;
      return;
    }

    if (sanitized->normalized == "main.lua") {
      hasMainLua = true;
    } else if (sanitized->normalized == "manifest.json") {
      hasManifest = true;
    }

    if (bundleFileCount >= kMaxBundleFiles) {
      LOG_ERR("APPS", "Install failed: bundle has too many files");
      extractOk = false;
      return;
    }

    if (entryPath.size() >= kMaxBundleEntryLen) {
      LOG_ERR("APPS", "Install failed: bundle entry path too long");
      extractOk = false;
      return;
    }

    BundleFileEntry& fileEntry = bundleFiles[bundleFileCount];
    memcpy(fileEntry.zipPath, entryPath.data(), entryPath.size());
    fileEntry.zipPath[entryPath.size()] = '\0';

    const int destWritten =
        snprintf(fileEntry.destPath, sizeof(fileEntry.destPath), "%s/%s", appDirPath, sanitized->normalized.c_str());
    if (destWritten <= 0 || static_cast<size_t>(destWritten) >= sizeof(fileEntry.destPath)) {
      LOG_ERR("APPS", "Install failed: path too long for %s", sanitized->normalized.c_str());
      extractOk = false;
      return;
    }

    bundleFileCount++;
  });

  for (size_t i = 0; extractOk && i < bundleFileCount; i++) {
    const BundleFileEntry& fileEntry = bundleFiles[i];
    if (!extractZipEntry(zip, fileEntry.zipPath, fileEntry.destPath)) {
      LOG_ERR("APPS", "Install failed: could not extract %s", fileEntry.zipPath);
      extractOk = false;
      break;
    }
  }

  Storage.remove(tmpZipPath);

  if (!extractOk || !hasMainLua || !hasManifest) {
    if (!hasMainLua || !hasManifest) {
      LOG_ERR("APPS", "Install failed: bundle missing required files");
    }
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
