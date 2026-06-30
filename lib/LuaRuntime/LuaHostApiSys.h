#pragma once

struct lua_State;

class LuaEngine;

void setActiveLuaEngine(LuaEngine* engine);
LuaEngine* getActiveLuaEngine();

void registerCpSysApi(lua_State* L, LuaEngine* engine);
