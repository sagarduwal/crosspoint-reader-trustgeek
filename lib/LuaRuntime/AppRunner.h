#pragma once

#include <string>

struct AppRunResult {
  bool success = false;
  bool exitRequested = false;
  std::string errorMessage;
};

// Executes an installed app's main.lua with heap limits and Host API bindings.
class AppRunner {
 public:
  static constexpr size_t kDefaultHeapCapKb = 64;

  static AppRunResult runMainLua(const std::string& mainLuaPath, size_t heapCapKb = kDefaultHeapCapKb);
};
