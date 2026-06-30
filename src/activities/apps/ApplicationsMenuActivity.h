#pragma once

#include <AppRegistryJson.h>

#include <vector>

#include "activities/Activity.h"
#include "util/ButtonNavigator.h"

class ApplicationsMenuActivity final : public Activity {
  ButtonNavigator buttonNavigator;
  int selectedIndex = 0;
  std::vector<AppRegistryEntry> entries_;

  static constexpr int kMenuItemCount = 1;

  void reloadEntries();
  [[nodiscard]] int totalItemCount() const;
  [[nodiscard]] static bool isMenuIndex(int index);
  [[nodiscard]] int appIndexFor(int index) const;

 public:
  explicit ApplicationsMenuActivity(GfxRenderer& renderer, MappedInputManager& mappedInput);

  void onEnter() override;
  void loop() override;
  void render(RenderLock&&) override;
};
