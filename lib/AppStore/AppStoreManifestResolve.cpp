#include "AppStoreManifestResolve.h"

#include <Logging.h>

#include "AppStoreManifestJson.h"

bool appStoreManifestResolveAndParse(const AppCatalogResolveInput& input, std::vector<AppCatalogEntry>& outEntries,
                                     uint8_t& outVersion, AppCatalogSource& outSource) {
  outSource = AppCatalogSource::None;
  outEntries.clear();
  outVersion = 0;

  if (input.remoteFetchOk && input.remoteJson != nullptr && input.remoteJson[0] != '\0') {
    if (AppStoreManifestJson::parse(input.remoteJson, outEntries, outVersion)) {
      outSource = AppCatalogSource::Remote;
      return true;
    }
    // #region agent log
    LOG_DBG("APPS", "[H4] remote JSON parse failed");
    // #endregion
  }

  if (input.cacheReadOk && input.cacheJson != nullptr && input.cacheJson[0] != '\0') {
    if (AppStoreManifestJson::parse(input.cacheJson, outEntries, outVersion)) {
      outSource = AppCatalogSource::Cache;
      return true;
    }
    // #region agent log
    LOG_DBG("APPS", "[H5] cache JSON parse failed");
    // #endregion
  }

  if (input.builtinJson != nullptr && input.builtinJson[0] != '\0') {
    if (AppStoreManifestJson::parse(input.builtinJson, outEntries, outVersion)) {
      outSource = AppCatalogSource::Builtin;
      return true;
    }
  }

  return false;
}
