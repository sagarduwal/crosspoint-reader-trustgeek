#pragma once

#include <string>

class GfxRenderer;
class MappedInputManager;

// Per-app Host API bindings context (set for the duration of AppRunner::runMainLua).
struct LuaHostApiContext {
  GfxRenderer* renderer = nullptr;
  MappedInputManager* mappedInput = nullptr;
  std::string appId;
  std::string displayName;
  int fontId = 0;
  int contentTop = 0;
  int contentHeight = 0;
};

void setActiveHostApiContext(LuaHostApiContext* context);
LuaHostApiContext* getActiveHostApiContext();

bool buildAppDataPath(const std::string& appId, const std::string& relativePath, std::string& absoluteOut);
bool buildAppBundlePath(const std::string& appId, const std::string& relativePath, std::string& absoluteOut);
