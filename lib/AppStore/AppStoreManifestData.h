#pragma once

namespace AppStoreManifestData {

constexpr char kRemoteUrl[] = "https://apps.crosspointreader.com/v1/manifest.json";

// Flash-resident catalog used when remote fetch and SD cache are unavailable.
constexpr char kBuiltinJson[] = R"({
  "version": 1,
  "apps": [
    {
      "id": "hello",
      "name": "Hello App",
      "version": "1.0.0",
      "description": "Smoke-test sample for the Lua runtime",
      "min_api_version": "1.0"
    }
  ]
})";

}  // namespace AppStoreManifestData
