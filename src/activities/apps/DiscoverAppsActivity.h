#pragma once

#include <AppCatalogEntry.h>
#include <AppRegistryJson.h>

#include <vector>

#include "activities/Activity.h"
#include "util/ButtonNavigator.h"

class DiscoverAppsActivity final : public Activity {
  ButtonNavigator buttonNavigator;
  int selectedIndex = 0;
  std::vector<AppCatalogEntry> entries_;
  std::vector<AppRegistryEntry> installedEntries_;
  bool hasAttemptedLoad_ = false;

  void loadCatalog();
  void onWifiSelectionComplete(bool success);
  bool installSelectedEntry();
  bool isInstalled(const std::string& appId) const;

 public:
  explicit DiscoverAppsActivity(GfxRenderer& renderer, MappedInputManager& mappedInput);

  void onEnter() override;
  void loop() override;
  void render(RenderLock&&) override;
};
