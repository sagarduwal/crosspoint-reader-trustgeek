#include "LuaHostApiSys.h"

#include "LuaEngine.h"

#include <Logging.h>

extern "C" {
#include "lauxlib.h"
#include "lua.h"
}

#include <Arduino.h>

namespace {

LuaEngine* currentEngine() { return getActiveLuaEngine(); }

int sysLog(lua_State* L) {
  const char* message = luaL_checkstring(L, 1);
  LOG_INF("LUA", "%s", message);
  return 0;
}

int sysExit(lua_State* L) {
  const int code = lua_gettop(L) >= 1 ? static_cast<int>(luaL_checkinteger(L, 1)) : 0;
  if (LuaEngine* engine = currentEngine()) {
    engine->requestExit();
  }
  lua_pushinteger(L, code);
  return 1;
}

int sysVersion(lua_State* L) {
  lua_pushstring(L, "1.0");
  return 1;
}

int sysDevice(lua_State* L) {
  lua_pushstring(L, "X4");
  return 1;
}

int sysBattery(lua_State* L) {
  lua_pushinteger(L, -1);
  return 1;
}

int sysMillis(lua_State* L) {
  lua_pushinteger(L, static_cast<lua_Integer>(millis()));
  return 1;
}

int sysSleepMs(lua_State* L) {
  const lua_Integer ms = luaL_checkinteger(L, 1);
  if (ms > 0) {
    delay(static_cast<unsigned long>(ms));
  }
  return 0;
}

const luaL_Reg kSysFunctions[] = {
    {"log", sysLog},
    {"exit", sysExit},
    {"version", sysVersion},
    {"device", sysDevice},
    {"battery", sysBattery},
    {"millis", sysMillis},
    {"sleep_ms", sysSleepMs},
    {nullptr, nullptr},
};

}  // namespace

void registerCpSysApi(lua_State* L, LuaEngine* engine) {
  setActiveLuaEngine(engine);
  luaL_newlib(L, kSysFunctions);
  lua_setfield(L, -2, "sys");
}
