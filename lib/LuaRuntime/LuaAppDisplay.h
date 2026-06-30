#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct LuaAppDisplayEntry {
  int16_t x = 0;
  int16_t y = 0;
  std::string text;
  bool centered = false;
};

// Framebuffer of text draws produced during a single app run (read by AppRunnerActivity).
class LuaAppDisplay {
 public:
  static constexpr size_t kMaxEntries = 48;
  static constexpr size_t kMaxTextLen = 96;

  static void clear();
  static bool addText(int x, int y, const char* text, bool centered);
  static const std::vector<LuaAppDisplayEntry>& entries();

 private:
  static std::vector<LuaAppDisplayEntry> entries_;
};
