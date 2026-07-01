#include "LuaHostApiContext.h"

#include <AppPathSanitizer.h>
#include <AppStorePaths.h>

#include <cstdio>

static LuaHostApiContext* gActiveContext = nullptr;

void setActiveHostApiContext(LuaHostApiContext* context) { gActiveContext = context; }

LuaHostApiContext* getActiveHostApiContext() { return gActiveContext; }

bool buildAppDataPath(const std::string& appId, const std::string& relativePath, std::string& absoluteOut) {
  if (!AppPathSanitizer::isValidAppId(appId)) {
    return false;
  }
  const auto sanitized = AppPathSanitizer::sanitizeRelativePath(relativePath);
  if (!sanitized.has_value()) {
    return false;
  }

  char buffer[160];
  const int written = snprintf(buffer, sizeof(buffer), "%s/%s/%s/%s", AppStorePaths::kAppsRoot, appId.c_str(),
                               AppStorePaths::kDataSubdir, sanitized->normalized.c_str());
  if (written <= 0 || static_cast<size_t>(written) >= sizeof(buffer)) {
    return false;
  }
  absoluteOut.assign(buffer);
  return true;
}

bool buildAppBundlePath(const std::string& appId, const std::string& relativePath, std::string& absoluteOut) {
  if (!AppPathSanitizer::isValidAppId(appId)) {
    return false;
  }
  const auto sanitized = AppPathSanitizer::sanitizeRelativePath(relativePath);
  if (!sanitized.has_value()) {
    return false;
  }

  char buffer[160];
  const int written = snprintf(buffer, sizeof(buffer), "%s/%s/%s", AppStorePaths::kAppsRoot, appId.c_str(),
                               sanitized->normalized.c_str());
  if (written <= 0 || static_cast<size_t>(written) >= sizeof(buffer)) {
    return false;
  }
  absoluteOut.assign(buffer);
  return true;
}
