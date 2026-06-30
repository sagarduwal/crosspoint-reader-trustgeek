#include "AppRunner.h"

#include "LuaEngine.h"

#include <Logging.h>

AppRunResult AppRunner::runMainLua(const std::string& mainLuaPath, size_t heapCapKb) {
  AppRunResult result;

  LuaEngine engine;
  const size_t heapCapBytes = heapCapKb * 1024;
  if (!engine.init(heapCapBytes)) {
    result.errorMessage = "Failed to initialize Lua runtime";
    LOG_ERR("APPS", "%s", result.errorMessage.c_str());
    return result;
  }

  if (!engine.loadAndRunFile(mainLuaPath.c_str(), result.errorMessage)) {
    LOG_ERR("APPS", "App run failed: %s", result.errorMessage.c_str());
    return result;
  }

  result.success = true;
  result.exitRequested = engine.exitRequested();
  return result;
}
