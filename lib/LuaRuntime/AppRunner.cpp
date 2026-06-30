#include "AppRunner.h"

#include "LuaEngine.h"
#include "LuaHostApiContext.h"

#include <Logging.h>

AppRunResult AppRunner::runMainLua(const std::string& mainLuaPath, const AppRunContext& context,
                                   const size_t heapCapKb) {
  AppRunResult result;

  LuaHostApiContext hostContext;
  hostContext.renderer = context.renderer;
  hostContext.mappedInput = context.mappedInput;
  hostContext.appId = context.appId;
  hostContext.displayName = context.displayName;
  hostContext.fontId = context.fontId;
  hostContext.contentTop = context.contentTop;
  hostContext.contentHeight = context.contentHeight;
  setActiveHostApiContext(&hostContext);

  LuaEngine engine;
  const size_t heapCapBytes = heapCapKb * 1024;
  if (!engine.init(heapCapBytes)) {
    setActiveHostApiContext(nullptr);
    result.errorMessage = "Failed to initialize Lua runtime";
    LOG_ERR("APPS", "%s", result.errorMessage.c_str());
    return result;
  }

  if (!engine.loadAndRunFile(mainLuaPath.c_str(), result.errorMessage)) {
    setActiveHostApiContext(nullptr);
    LOG_ERR("APPS", "App run failed: %s", result.errorMessage.c_str());
    return result;
  }

  setActiveHostApiContext(nullptr);
  result.success = true;
  result.exitRequested = engine.exitRequested();
  return result;
}
