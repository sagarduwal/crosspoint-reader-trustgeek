#pragma once

#include <cstddef>
#include <string>

struct lua_State;

// Owns a Lua 5.4 state with sandboxed standard libraries and cp.* Host API bindings.
class LuaEngine {
 public:
  static constexpr size_t kDefaultHeapCapBytes = 64 * 1024;

  LuaEngine();
  ~LuaEngine();

  LuaEngine(const LuaEngine&) = delete;
  LuaEngine& operator=(const LuaEngine&) = delete;

  bool init(size_t heapCapBytes = kDefaultHeapCapBytes);
  void close();

  bool isOpen() const { return state_ != nullptr; }
  lua_State* state() const { return state_; }

  bool loadAndRunFile(const char* path, std::string& errorOut);
  bool loadStringAndRun(const char* chunkName, const char* source, std::string& errorOut);

  void requestExit() { exitRequested_ = true; }
  bool exitRequested() const { return exitRequested_; }

 private:
  void openSandboxedLibraries();
  void registerHostApi();

  lua_State* state_ = nullptr;
  class LuaAllocator* allocator_ = nullptr;
  bool exitRequested_ = false;
};
