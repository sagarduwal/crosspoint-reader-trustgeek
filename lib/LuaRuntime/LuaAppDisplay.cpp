#include "LuaAppDisplay.h"

#include <Logging.h>

#include <cstring>

std::vector<LuaAppDisplayEntry> LuaAppDisplay::entries_;

void LuaAppDisplay::clear() { entries_.clear(); }

bool LuaAppDisplay::addText(const int x, const int y, const char* text, const bool centered) {
  if (text == nullptr) {
    // #region agent log
    LOG_DBG("APPS", "display addText rejected: null text");
    // #endregion
    return false;
  }

  const size_t len = strnlen(text, kMaxTextLen);
  if (len == 0) {
    // Blank lines are spacing-only; caller advances y separately.
    return true;
  }

  if (entries_.size() >= kMaxEntries) {
    // #region agent log
    LOG_DBG("APPS", "display addText rejected: buffer full entries=%u", static_cast<unsigned>(entries_.size()));
    // #endregion
    return false;
  }

  LuaAppDisplayEntry entry;
  entry.x = static_cast<int16_t>(x);
  entry.y = static_cast<int16_t>(y);
  entry.centered = centered;
  entry.text.assign(text, len);

  entries_.push_back(std::move(entry));
  return true;
}

const std::vector<LuaAppDisplayEntry>& LuaAppDisplay::entries() { return entries_; }
