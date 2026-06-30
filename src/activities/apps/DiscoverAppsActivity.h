#pragma once

#include <AppCatalogEntry.h>

#include <vector>

#include "activities/Activity.h"
#include "util/ButtonNavigator.h"

class DiscoverAppsActivity final : public Activity {
  ButtonNavigator buttonNavigator;
  int selectedIndex = 0;
  std::vector<AppCatalogEntry> entries_;

 public:
  explicit DiscoverAppsActivity(GfxRenderer& renderer, MappedInputManager& mappedInput);

  void onEnter() override;
  void loop() override;
  void render(RenderLock&&) override;
};
