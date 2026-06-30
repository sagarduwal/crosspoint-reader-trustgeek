#pragma once

#include <string>

// One app listed in the Discover catalog (remote manifest or builtin fallback).
struct AppCatalogEntry {
  std::string id;
  std::string name;
  std::string version;
  std::string description;
  std::string bundleUrl;
  std::string sha256;
  std::string minApiVersion;
  std::string minFirmware;
};
