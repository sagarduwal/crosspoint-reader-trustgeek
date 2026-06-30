#include "LuaHostApiSys.h"

static LuaEngine* gActiveEngine = nullptr;

void setActiveLuaEngine(LuaEngine* engine) { gActiveEngine = engine; }

LuaEngine* getActiveLuaEngine() { return gActiveEngine; }
