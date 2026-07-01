#pragma once

#include <cstdint>
#include <string>
#include <vector>

class GfxRenderer;

struct LuaAppDisplayEntry {
  int16_t x = 0;
  int16_t y = 0;
  std::string text;
  bool centered = false;
};

struct LuaAppBitmapEntry {
  std::string bundleRelativePath;
  int16_t x = 0;
  int16_t y = 0;
  int16_t maxW = 0;
  int16_t maxH = 0;
};

// Framebuffer of draws produced during a single app run (read by AppRunnerActivity).
class LuaAppDisplay {
 public:
  static constexpr size_t kMaxEntries = 48;
  static constexpr size_t kMaxBitmapEntries = 4;
  static constexpr size_t kMaxTextLen = 96;
  static constexpr size_t kMaxBundlePathLen = 96;

  static void clear();
  static bool addText(int x, int y, const char* text, bool centered);
  static bool addBitmap(int x, int y, const char* bundleRelativePath, int maxW, int maxH);
  static const std::vector<LuaAppDisplayEntry>& entries();
  static const std::vector<LuaAppBitmapEntry>& bitmapEntries();
  static void paint(GfxRenderer& renderer, int fontId, const std::string& appId, int contentTop);

 private:
  static std::vector<LuaAppDisplayEntry> entries_;
  static std::vector<LuaAppBitmapEntry> bitmapEntries_;
};
