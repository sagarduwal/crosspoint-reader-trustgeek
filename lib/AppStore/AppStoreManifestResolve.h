#pragma once

#include "AppCatalogEntry.h"
#include "AppStoreManifestTypes.h"

#include <cstdint>
#include <vector>

bool appStoreManifestResolveAndParse(const AppCatalogResolveInput& input, std::vector<AppCatalogEntry>& outEntries,
                                     uint8_t& outVersion, AppCatalogSource& outSource);
