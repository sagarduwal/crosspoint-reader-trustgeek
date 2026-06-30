#include "LuaAppDisplay.h"

#include <cstring>

std::vector<LuaAppDisplayEntry> LuaAppDisplay::entries_;

void LuaAppDisplay::clear() { entries_.clear(); }

bool LuaAppDisplay::addText(const int x, const int y, const char* text, const bool centered) {
  if (text == nullptr || entries_.size() >= kMaxEntries) {
    return false;
  }

  LuaAppDisplayEntry entry;
  entry.x = static_cast<int16_t>(x);
  entry.y = static_cast<int16_t>(y);
  entry.centered = centered;

  const size_t len = strnlen(text, kMaxTextLen);
  entry.text.assign(text, len);
  if (entry.text.empty()) {
    return false;
  }

  entries_.push_back(std::move(entry));
  return true;
}

const std::vector<LuaAppDisplayEntry>& LuaAppDisplay::entries() { return entries_; }
