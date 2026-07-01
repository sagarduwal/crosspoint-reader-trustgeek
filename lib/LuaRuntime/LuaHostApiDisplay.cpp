#include "LuaHostApiDisplay.h"

#include "LuaAppDisplay.h"
#include "LuaHostApiContext.h"

#include <GfxRenderer.h>
#include <HalStorage.h>

extern "C" {
#include "lauxlib.h"
#include "lua.h"
}

namespace {

void paintBufferedDisplay(GfxRenderer& renderer, const int fontId, const std::string& appId, const int contentTop) {
  renderer.clearScreen();
  LuaAppDisplay::paint(renderer, fontId, appId, contentTop);
  renderer.displayBuffer();
}

int displayClear(lua_State* L) {
  (void)L;
  LuaAppDisplay::clear();
  return 0;
}

int displayText(lua_State* L) {
  const int x = static_cast<int>(luaL_checkinteger(L, 1));
  const int y = static_cast<int>(luaL_checkinteger(L, 2));
  const char* text = luaL_checkstring(L, 3);
  if (!LuaAppDisplay::addText(x, y, text, false)) {
    return luaL_error(L, "display entry limit reached");
  }
  return 0;
}

int displayCenter(lua_State* L) {
  const int y = static_cast<int>(luaL_checkinteger(L, 1));
  const char* text = luaL_checkstring(L, 2);
  if (!LuaAppDisplay::addText(0, y, text, true)) {
    return luaL_error(L, "display entry limit reached");
  }
  return 0;
}

int displayBmp(lua_State* L) {
  const char* path = luaL_checkstring(L, 1);
  const int x = static_cast<int>(luaL_checkinteger(L, 2));
  const int y = static_cast<int>(luaL_checkinteger(L, 3));
  const int maxW = lua_gettop(L) >= 4 ? static_cast<int>(luaL_checkinteger(L, 4)) : 0;
  const int maxH = lua_gettop(L) >= 5 ? static_cast<int>(luaL_checkinteger(L, 5)) : 0;

  LuaHostApiContext* context = getActiveHostApiContext();
  if (context == nullptr || context->appId.empty()) {
    return luaL_error(L, "display unavailable");
  }

  std::string absolutePath;
  if (!buildAppBundlePath(context->appId, path, absolutePath)) {
    return luaL_error(L, "invalid asset path");
  }
  if (!Storage.exists(absolutePath.c_str())) {
    return luaL_error(L, "asset not found: %s", path);
  }

  if (!LuaAppDisplay::addBitmap(x, y, path, maxW, maxH)) {
    return luaL_error(L, "display bitmap limit reached");
  }
  return 0;
}

int displayWidth(lua_State* L) {
  LuaHostApiContext* context = getActiveHostApiContext();
  if (context == nullptr || context->renderer == nullptr) {
    return luaL_error(L, "display unavailable");
  }
  lua_pushinteger(L, context->renderer->getScreenWidth());
  return 1;
}

int displayHeight(lua_State* L) {
  LuaHostApiContext* context = getActiveHostApiContext();
  if (context == nullptr || context->renderer == nullptr) {
    return luaL_error(L, "display unavailable");
  }
  lua_pushinteger(L, context->renderer->getScreenHeight());
  return 1;
}

int displayContentTop(lua_State* L) {
  LuaHostApiContext* context = getActiveHostApiContext();
  if (context == nullptr) {
    return luaL_error(L, "display unavailable");
  }
  lua_pushinteger(L, context->contentTop);
  return 1;
}

int displayContentHeight(lua_State* L) {
  LuaHostApiContext* context = getActiveHostApiContext();
  if (context == nullptr) {
    return luaL_error(L, "display unavailable");
  }
  lua_pushinteger(L, context->contentHeight);
  return 1;
}

int displayRefresh(lua_State* L) {
  LuaHostApiContext* context = getActiveHostApiContext();
  if (context == nullptr || context->renderer == nullptr) {
    return luaL_error(L, "display unavailable");
  }
  if (context->fontId == 0) {
    return luaL_error(L, "display font not configured");
  }

  paintBufferedDisplay(*context->renderer, context->fontId, context->appId, 0);
  LuaAppDisplay::clear();
  return 0;
}

const luaL_Reg kDisplayFunctions[] = {
    {"clear", displayClear},
    {"text", displayText},
    {"center", displayCenter},
    {"bmp", displayBmp},
    {"width", displayWidth},
    {"height", displayHeight},
    {"content_top", displayContentTop},
    {"content_height", displayContentHeight},
    {"refresh", displayRefresh},
    {nullptr, nullptr},
};

}  // namespace

void registerCpDisplayApi(lua_State* L) {
  luaL_newlib(L, kDisplayFunctions);
  lua_setfield(L, -2, "display");
}
