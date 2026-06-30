#pragma once

#include <cstddef>
#include <cstdint>

// Tracks Lua heap usage against a fixed cap (default 64 KB per PRD).
class LuaAllocator {
 public:
  explicit LuaAllocator(size_t capBytes);

  size_t capBytes() const { return capBytes_; }
  size_t usedBytes() const { return usedBytes_; }

  void* realloc(void* ptr, size_t oldSize, size_t newSize);

 private:
  size_t capBytes_;
  size_t usedBytes_ = 0;
};

// C callback for lua_Alloc.
void* luaCapAllocator(void* ud, void* ptr, size_t osize, size_t nsize);
