#include "LuaHostApiSettings.h"

#include "LuaHostApiContext.h"

#include <ArduinoJson.h>
#include <AppStorePaths.h>
#include <CrossPointSettings.h>
#include <HalStorage.h>

#include <cstring>

extern "C" {
#include "lauxlib.h"
#include "lua.h"
}

namespace {

constexpr char kSettingsFile[] = "settings.json";
constexpr size_t kSettingsCapacity = 512;

bool ensureAppDataDir(const std::string& appId);

bool readSettings(JsonDocument& doc) {
  LuaHostApiContext* context = getActiveHostApiContext();
  if (context == nullptr || context->appId.empty()) {
    return false;
  }

  std::string absolutePath;
  if (!buildAppDataPath(context->appId, kSettingsFile, absolutePath)) {
    return false;
  }

  if (!Storage.exists(absolutePath.c_str())) {
    doc.to<JsonObject>();
    return true;
  }

  const String content = Storage.readFile(absolutePath.c_str());
  if (content.isEmpty()) {
    doc.to<JsonObject>();
    return true;
  }

  const DeserializationError error = deserializeJson(doc, content.c_str());
  if (error) {
    doc.to<JsonObject>();
    return false;
  }
  return true;
}

bool writeSettings(const JsonDocument& doc) {
  LuaHostApiContext* context = getActiveHostApiContext();
  if (context == nullptr || context->appId.empty()) {
    return false;
  }

  if (!ensureAppDataDir(context->appId)) {
    return false;
  }

  std::string absolutePath;
  if (!buildAppDataPath(context->appId, kSettingsFile, absolutePath)) {
    return false;
  }

  char buffer[kSettingsCapacity];
  const size_t length = serializeJson(doc, buffer, sizeof(buffer));
  if (length == 0 || length >= sizeof(buffer)) {
    return false;
  }

  char tmpPath[168];
  snprintf(tmpPath, sizeof(tmpPath), "%s.tmp", absolutePath.c_str());
  const String payload(buffer, length);
  if (!Storage.writeFile(tmpPath, payload)) {
    return false;
  }
  if (!Storage.rename(tmpPath, absolutePath.c_str())) {
    Storage.remove(tmpPath);
    return false;
  }
  return true;
}

bool ensureAppDataDir(const std::string& appId) {
  char dirPath[128];
  const int written = snprintf(dirPath, sizeof(dirPath), "%s/%s/%s", AppStorePaths::kAppsRoot, appId.c_str(),
                               AppStorePaths::kDataSubdir);
  if (written <= 0 || static_cast<size_t>(written) >= sizeof(dirPath)) {
    return false;
  }
  return Storage.ensureDirectoryExists(dirPath);
}

void pushSettingValue(lua_State* L, const JsonVariantConst value) {
  if (value.is<int>()) {
    lua_pushinteger(L, value.as<int>());
  } else if (value.is<const char*>()) {
    lua_pushstring(L, value.as<const char*>());
  } else if (value.is<bool>()) {
    lua_pushboolean(L, value.as<bool>() ? 1 : 0);
  } else {
    lua_pushnil(L);
  }
}

int settingsGet(lua_State* L) {
  const char* key = luaL_checkstring(L, 1);

  StaticJsonDocument<kSettingsCapacity> doc;
  readSettings(doc);

  const JsonVariantConst value = doc[key];
  if (value.isNull()) {
    if (lua_gettop(L) >= 2) {
      lua_pushvalue(L, 2);
      return 1;
    }
    lua_pushnil(L);
    return 1;
  }

  pushSettingValue(L, value);
  return 1;
}

int settingsSet(lua_State* L) {
  const char* key = luaL_checkstring(L, 1);
  luaL_checkany(L, 2);

  StaticJsonDocument<kSettingsCapacity> doc;
  readSettings(doc);

  if (lua_isinteger(L, 2) || lua_isnumber(L, 2)) {
    doc[key] = static_cast<int>(lua_tointeger(L, 2));
  } else if (lua_isboolean(L, 2)) {
    doc[key] = lua_toboolean(L, 2) != 0;
  } else if (lua_isstring(L, 2)) {
    doc[key] = lua_tostring(L, 2);
  } else {
    return luaL_error(L, "unsupported settings value type");
  }

  if (!writeSettings(doc)) {
    lua_pushboolean(L, 0);
    lua_pushstring(L, "write failed");
    return 2;
  }

  lua_pushboolean(L, 1);
  return 1;
}

int settingsGlobalGet(lua_State* L) {
  const char* key = luaL_checkstring(L, 1);
  if (strcmp(key, "clock_utc_offset_q") == 0) {
    lua_pushinteger(L, SETTINGS.clockUtcOffsetQ);
    return 1;
  }
  if (strcmp(key, "clock_format") == 0) {
    lua_pushinteger(L, SETTINGS.clockFormat);
    return 1;
  }
  if (strcmp(key, "clock_synced") == 0) {
    lua_pushboolean(L, SETTINGS.clockHasBeenSynced != 0 ? 1 : 0);
    return 1;
  }
  if (lua_gettop(L) >= 2) {
    lua_pushvalue(L, 2);
    return 1;
  }
  lua_pushnil(L);
  return 1;
}

const luaL_Reg kSettingsFunctions[] = {
    {"get", settingsGet},
    {"set", settingsSet},
    {"global_get", settingsGlobalGet},
    {nullptr, nullptr},
};

}  // namespace

void registerCpSettingsApi(lua_State* L) {
  luaL_newlib(L, kSettingsFunctions);
  lua_setfield(L, -2, "settings");
}
