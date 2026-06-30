#include "LuaHostApiDisplay.h"

#include "LuaAppDisplay.h"

extern "C" {
#include "lauxlib.h"
#include "lua.h"
}

namespace {

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
    return luaL_error(L, "display buffer full");
  }
  return 0;
}

int displayCenter(lua_State* L) {
  const int y = static_cast<int>(luaL_checkinteger(L, 1));
  const char* text = luaL_checkstring(L, 2);
  if (!LuaAppDisplay::addText(0, y, text, true)) {
    return luaL_error(L, "display buffer full");
  }
  return 0;
}

const luaL_Reg kDisplayFunctions[] = {
    {"clear", displayClear},
    {"text", displayText},
    {"center", displayCenter},
    {nullptr, nullptr},
};

}  // namespace

void registerCpDisplayApi(lua_State* L) {
  luaL_newlib(L, kDisplayFunctions);
  lua_setfield(L, -2, "display");
}
