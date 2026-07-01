#include "LuaAppDisplay.h"

#include "LuaHostApiContext.h"

#include <Bitmap.h>
#include <GfxRenderer.h>
#include <HalStorage.h>
#include <Logging.h>

#include <cstring>

std::vector<LuaAppDisplayEntry> LuaAppDisplay::entries_;
std::vector<LuaAppBitmapEntry> LuaAppDisplay::bitmapEntries_;

void LuaAppDisplay::clear() {
  entries_.clear();
  bitmapEntries_.clear();
}

bool LuaAppDisplay::addText(const int x, const int y, const char* text, const bool centered) {
  if (text == nullptr) {
    LOG_DBG("APPS", "display addText rejected: null text");
    return false;
  }

  const size_t len = strnlen(text, kMaxTextLen);
  if (len == 0) {
    return true;
  }

  if (entries_.size() >= kMaxEntries) {
    LOG_DBG("APPS", "display addText rejected: buffer full entries=%u", static_cast<unsigned>(entries_.size()));
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

bool LuaAppDisplay::addBitmap(const int x, const int y, const char* bundleRelativePath, const int maxW,
                              const int maxH) {
  if (bundleRelativePath == nullptr) {
    return false;
  }

  const size_t len = strnlen(bundleRelativePath, kMaxBundlePathLen);
  if (len == 0) {
    return false;
  }

  if (bitmapEntries_.size() >= kMaxBitmapEntries) {
    LOG_DBG("APPS", "display addBitmap rejected: buffer full");
    return false;
  }

  LuaAppBitmapEntry entry;
  entry.x = static_cast<int16_t>(x);
  entry.y = static_cast<int16_t>(y);
  entry.maxW = static_cast<int16_t>(maxW);
  entry.maxH = static_cast<int16_t>(maxH);
  entry.bundleRelativePath.assign(bundleRelativePath, len);

  bitmapEntries_.push_back(std::move(entry));
  return true;
}

const std::vector<LuaAppDisplayEntry>& LuaAppDisplay::entries() { return entries_; }

const std::vector<LuaAppBitmapEntry>& LuaAppDisplay::bitmapEntries() { return bitmapEntries_; }

void LuaAppDisplay::paint(GfxRenderer& renderer, const int fontId, const std::string& appId, const int contentTop) {
  for (const LuaAppBitmapEntry& bmpEntry : bitmapEntries_) {
    std::string absolutePath;
    if (!buildAppBundlePath(appId, bmpEntry.bundleRelativePath, absolutePath)) {
      LOG_ERR("APPS", "display bmp rejected: invalid path %s", bmpEntry.bundleRelativePath.c_str());
      continue;
    }

    HalFile file;
    if (!Storage.openFileForRead("APPS", absolutePath.c_str(), file)) {
      LOG_ERR("APPS", "display bmp missing: %s", absolutePath.c_str());
      continue;
    }

    Bitmap bitmap(file);
    if (bitmap.parseHeaders() != BmpReaderError::Ok) {
      LOG_ERR("APPS", "display bmp parse failed: %s", absolutePath.c_str());
      continue;
    }

    const int drawY = contentTop + bmpEntry.y;
    const int maxW = bmpEntry.maxW > 0 ? bmpEntry.maxW : bitmap.getWidth();
    const int maxH = bmpEntry.maxH > 0 ? bmpEntry.maxH : bitmap.getHeight();
    renderer.drawBitmap(bitmap, bmpEntry.x, drawY, maxW, maxH);
  }

  for (const LuaAppDisplayEntry& entry : entries_) {
    const int drawY = contentTop + entry.y;
    if (entry.centered) {
      renderer.drawCenteredText(fontId, drawY, entry.text.c_str());
    } else {
      renderer.drawText(fontId, entry.x, drawY, entry.text.c_str());
    }
  }
}
