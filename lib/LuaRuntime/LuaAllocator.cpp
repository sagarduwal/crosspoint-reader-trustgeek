#include "LuaAllocator.h"

#include <cstdlib>
#include <cstring>

LuaAllocator::LuaAllocator(size_t capBytes) : capBytes_(capBytes) {}

void* LuaAllocator::realloc(void* ptr, size_t oldSize, size_t newSize) {
  if (newSize == 0) {
    if (ptr != nullptr) {
      usedBytes_ -= oldSize;
      std::free(ptr);
    }
    return nullptr;
  }

  if (ptr == nullptr) {
    if (usedBytes_ + newSize > capBytes_) {
      return nullptr;
    }
    void* block = std::malloc(newSize);
    if (block != nullptr) {
      usedBytes_ += newSize;
    }
    return block;
  }

  if (newSize <= oldSize) {
    usedBytes_ -= (oldSize - newSize);
    return std::realloc(ptr, newSize);
  }

  const size_t delta = newSize - oldSize;
  if (usedBytes_ + delta > capBytes_) {
    return nullptr;
  }
  void* block = std::realloc(ptr, newSize);
  if (block != nullptr) {
    usedBytes_ += delta;
  }
  return block;
}

void* luaCapAllocator(void* ud, void* ptr, size_t osize, size_t nsize) {
  return static_cast<LuaAllocator*>(ud)->realloc(ptr, osize, nsize);
}
