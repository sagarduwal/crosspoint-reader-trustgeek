#include "LuaHostApiFs.h"

#include "LuaHostApiContext.h"

#include <AppStorePaths.h>
#include <HalStorage.h>
#include <Logging.h>

extern "C" {
#include "lauxlib.h"
#include "lua.h"
}

namespace {

bool ensureAppDataDir(const std::string& appId) {
  char dirPath[128];
  const int written =
      snprintf(dirPath, sizeof(dirPath), "%s/%s/%s", AppStorePaths::kAppsRoot, appId.c_str(), AppStorePaths::kDataSubdir);
  if (written <= 0 || static_cast<size_t>(written) >= sizeof(dirPath)) {
    return false;
  }
  return Storage.ensureDirectoryExists(dirPath);
}

int fsRead(lua_State* L) {
  LuaHostApiContext* context = getActiveHostApiContext();
  if (context == nullptr || context->appId.empty()) {
    return luaL_error(L, "filesystem unavailable");
  }

  const char* relativePath = luaL_checkstring(L, 1);
  std::string absolutePath;
  if (!buildAppDataPath(context->appId, relativePath, absolutePath)) {
    lua_pushnil(L);
    lua_pushstring(L, "invalid path");
    return 2;
  }

  if (!Storage.exists(absolutePath.c_str())) {
    lua_pushnil(L);
    lua_pushstring(L, "file not found");
    return 2;
  }

  const String content = Storage.readFile(absolutePath.c_str());
  lua_pushlstring(L, content.c_str(), content.length());
  return 1;
}

int fsWrite(lua_State* L) {
  LuaHostApiContext* context = getActiveHostApiContext();
  if (context == nullptr || context->appId.empty()) {
    return luaL_error(L, "filesystem unavailable");
  }

  const char* relativePath = luaL_checkstring(L, 1);
  size_t length = 0;
  const char* data = luaL_checklstring(L, 2, &length);

  std::string absolutePath;
  if (!buildAppDataPath(context->appId, relativePath, absolutePath)) {
    lua_pushboolean(L, 0);
    lua_pushstring(L, "invalid path");
    return 2;
  }

  if (!ensureAppDataDir(context->appId)) {
    LOG_ERR("LUA", "Failed to ensure app data dir for %s", context->appId.c_str());
    lua_pushboolean(L, 0);
    lua_pushstring(L, "mkdir failed");
    return 2;
  }

  const String payload(data, length);
  if (!Storage.writeFile(absolutePath.c_str(), payload)) {
    lua_pushboolean(L, 0);
    lua_pushstring(L, "write failed");
    return 2;
  }

  lua_pushboolean(L, 1);
  return 1;
}

int fsExists(lua_State* L) {
  LuaHostApiContext* context = getActiveHostApiContext();
  if (context == nullptr || context->appId.empty()) {
    return luaL_error(L, "filesystem unavailable");
  }

  const char* relativePath = luaL_checkstring(L, 1);
  std::string absolutePath;
  if (!buildAppDataPath(context->appId, relativePath, absolutePath)) {
    lua_pushboolean(L, 0);
    return 1;
  }

  lua_pushboolean(L, Storage.exists(absolutePath.c_str()));
  return 1;
}

const luaL_Reg kFsFunctions[] = {
    {"read", fsRead},
    {"write", fsWrite},
    {"exists", fsExists},
    {nullptr, nullptr},
};

}  // namespace

void registerCpFsApi(lua_State* L) {
  luaL_newlib(L, kFsFunctions);
  lua_setfield(L, -2, "fs");
}
