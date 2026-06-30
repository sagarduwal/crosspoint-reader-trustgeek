#pragma once

#include <cstdint>

enum class AppCatalogSource {
  None = 0,
  Remote,
  Cache,
  Builtin,
};

struct AppCatalogResolveInput {
  const char* remoteJson = nullptr;
  bool remoteFetchOk = false;
  const char* cacheJson = nullptr;
  bool cacheReadOk = false;
  const char* builtinJson = nullptr;
};
