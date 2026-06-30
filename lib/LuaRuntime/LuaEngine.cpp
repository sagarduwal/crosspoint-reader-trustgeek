#include "LuaEngine.h"

#include "LuaAllocator.h"
#include "LuaAppDisplay.h"
#include "LuaHostApiDisplay.h"
#include "LuaHostApiSys.h"

#include <HalStorage.h>
#include <Logging.h>

#include <cstring>

extern "C" {
#include "lauxlib.h"
#include "lualib.h"
#include "lua.h"
}

namespace {

bool pushLuaError(lua_State* L, std::string& errorOut) {
  const char* message = lua_tostring(L, -1);
  errorOut = message != nullptr ? message : "unknown Lua error";
  lua_pop(L, 1);
  return false;
}

}  // namespace

LuaEngine::LuaEngine() = default;

LuaEngine::~LuaEngine() { close(); }

bool LuaEngine::init(size_t heapCapBytes) {
  close();
  exitRequested_ = false;

  allocator_ = new (std::nothrow) LuaAllocator(heapCapBytes);
  if (allocator_ == nullptr) {
    LOG_ERR("LUA", "OOM: LuaAllocator");
    return false;
  }

  state_ = lua_newstate(luaCapAllocator, allocator_);
  if (state_ == nullptr) {
    LOG_ERR("LUA", "Failed to create Lua state");
    delete allocator_;
    allocator_ = nullptr;
    return false;
  }

  openSandboxedLibraries();
  registerHostApi();
  return true;
}

void LuaEngine::close() {
  if (state_ != nullptr) {
    setActiveLuaEngine(nullptr);
    lua_close(state_);
    state_ = nullptr;
  }
  if (allocator_ != nullptr) {
    delete allocator_;
    allocator_ = nullptr;
  }
}

void LuaEngine::openSandboxedLibraries() {
  luaL_requiref(state_, LUA_GNAME, luaopen_base, 1);
  lua_pop(state_, 1);
  luaL_requiref(state_, LUA_TABLIBNAME, luaopen_table, 1);
  lua_pop(state_, 1);
  luaL_requiref(state_, LUA_STRLIBNAME, luaopen_string, 1);
  lua_pop(state_, 1);
  luaL_requiref(state_, LUA_MATHLIBNAME, luaopen_math, 1);
  lua_pop(state_, 1);
  luaL_requiref(state_, LUA_COLIBNAME, luaopen_coroutine, 1);
  lua_pop(state_, 1);
}

void LuaEngine::registerHostApi() {
  LuaAppDisplay::clear();
  lua_newtable(state_);
  registerCpSysApi(state_, this);
  registerCpDisplayApi(state_);
  lua_setglobal(state_, "cp");
}

bool LuaEngine::loadAndRunFile(const char* path, std::string& errorOut) {
  if (state_ == nullptr) {
    errorOut = "Lua state not initialized";
    return false;
  }

  if (!Storage.exists(path)) {
    errorOut = std::string("cannot open ") + path + ": No such file or directory";
    return false;
  }

  const String script = Storage.readFile(path);
  if (script.isEmpty()) {
    errorOut = std::string("cannot read ") + path;
    return false;
  }

  const int loadStatus = luaL_loadbuffer(state_, script.c_str(), script.length(), path);
  if (loadStatus != LUA_OK) {
    return pushLuaError(state_, errorOut);
  }

  const int callStatus = lua_pcall(state_, 0, 0, 0);
  if (callStatus != LUA_OK) {
    return pushLuaError(state_, errorOut);
  }
  return true;
}

bool LuaEngine::loadStringAndRun(const char* chunkName, const char* source, std::string& errorOut) {
  if (state_ == nullptr) {
    errorOut = "Lua state not initialized";
    return false;
  }

  const int loadStatus = luaL_loadbuffer(state_, source, strlen(source), chunkName);
  if (loadStatus != LUA_OK) {
    return pushLuaError(state_, errorOut);
  }

  const int callStatus = lua_pcall(state_, 0, 0, 0);
  if (callStatus != LUA_OK) {
    return pushLuaError(state_, errorOut);
  }
  return true;
}
