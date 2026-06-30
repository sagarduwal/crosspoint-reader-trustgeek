#pragma once

#include <string>

class GfxRenderer;
class MappedInputManager;

struct AppRunResult {
  bool success = false;
  bool exitRequested = false;
  std::string errorMessage;
};

struct AppRunContext {
  GfxRenderer* renderer = nullptr;
  MappedInputManager* mappedInput = nullptr;
  std::string appId;
  std::string displayName;
  int fontId = 0;
  int contentTop = 0;
  int contentHeight = 0;
};

// Executes an installed app's main.lua with heap limits and Host API bindings.
class AppRunner {
 public:
  static constexpr size_t kDefaultHeapCapKb = 64;

  static AppRunResult runMainLua(const std::string& mainLuaPath, const AppRunContext& context = {},
                                 size_t heapCapKb = kDefaultHeapCapKb);
};
