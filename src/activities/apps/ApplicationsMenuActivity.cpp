#include "ApplicationsMenuActivity.h"

#include <AppRegistry.h>
#include <GfxRenderer.h>
#include <I18n.h>

#include <algorithm>

#include "AppRunnerActivity.h"
#include "DiscoverAppsActivity.h"
#include "MappedInputManager.h"
#include "components/UITheme.h"
#include "fontIds.h"

ApplicationsMenuActivity::ApplicationsMenuActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
    : Activity("Applications", renderer, mappedInput) {}

void ApplicationsMenuActivity::reloadEntries() {
  APP_REGISTRY.loadFromFile();
  entries_ = APP_REGISTRY.getEntries();
  std::sort(entries_.begin(), entries_.end(),
            [](const AppRegistryEntry& a, const AppRegistryEntry& b) { return a.name < b.name; });
}

int ApplicationsMenuActivity::totalItemCount() const {
  return kMenuItemCount + static_cast<int>(entries_.size());
}

bool ApplicationsMenuActivity::isMenuIndex(const int index) { return index < kMenuItemCount; }

int ApplicationsMenuActivity::appIndexFor(const int index) const { return index - kMenuItemCount; }

void ApplicationsMenuActivity::onEnter() {
  Activity::onEnter();
  reloadEntries();
  selectedIndex = 0;
  requestUpdate();
}

void ApplicationsMenuActivity::loop() {
  if (mappedInput.wasPressed(MappedInputManager::Button::Back)) {
    onGoHome(HomeMenuItem::APPLICATIONS);
    return;
  }

  const int itemCount = totalItemCount();

  if (mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
    if (isMenuIndex(selectedIndex)) {
      activityManager.replaceActivity(std::make_unique<DiscoverAppsActivity>(renderer, mappedInput));
      return;
    }

    const int appIndex = appIndexFor(selectedIndex);
    if (appIndex < 0 || appIndex >= static_cast<int>(entries_.size())) {
      return;
    }
    const std::string appId = entries_[static_cast<size_t>(appIndex)].id;
    const std::string& appName = entries_[static_cast<size_t>(appIndex)].name;
    startActivityForResult(std::make_unique<AppRunnerActivity>(renderer, mappedInput, appId, appName),
                           [this](const ActivityResult&) {
                             reloadEntries();
                             const int count = totalItemCount();
                             if (selectedIndex >= count) {
                               selectedIndex = count - 1;
                             }
                             if (selectedIndex < 0) {
                               selectedIndex = 0;
                             }
                             requestUpdate();
                           });
    return;
  }

  buttonNavigator.onNextRelease([this, itemCount] {
    selectedIndex = ButtonNavigator::nextIndex(selectedIndex, itemCount);
    requestUpdate();
  });

  buttonNavigator.onPreviousRelease([this, itemCount] {
    selectedIndex = ButtonNavigator::previousIndex(selectedIndex, itemCount);
    requestUpdate();
  });
}

void ApplicationsMenuActivity::render(RenderLock&&) {
  renderer.clearScreen();

  const auto& metrics = UITheme::getInstance().getMetrics();
  const auto pageWidth = renderer.getScreenWidth();
  const auto pageHeight = renderer.getScreenHeight();

  GUI.drawHeader(renderer, Rect{0, metrics.topPadding, pageWidth, metrics.headerHeight}, tr(STR_APPLICATIONS));

  const int contentTop = metrics.topPadding + metrics.headerHeight + metrics.verticalSpacing;
  const int contentBottom = pageHeight - metrics.buttonHintsHeight - metrics.verticalSpacing * 2;

  const int menuRowHeight = metrics.listRowHeight;
  const int menuHeight = kMenuItemCount * menuRowHeight;
  const int separatorY = contentTop + menuHeight + metrics.verticalSpacing / 2;
  const int appsTop = contentTop + menuHeight + metrics.verticalSpacing;
  const int appsHeight = contentBottom - appsTop;

  const int menuSelected = isMenuIndex(selectedIndex) ? selectedIndex : -1;
  GUI.drawList(renderer, Rect{0, contentTop, pageWidth, menuHeight}, kMenuItemCount, menuSelected,
               [](int index) {
                 (void)index;
                 return std::string(tr(STR_DISCOVER_APPS));
               },
               nullptr);

  renderer.drawLine(0, separatorY, pageWidth - 1, separatorY, 3, true);

  const int appCount = static_cast<int>(entries_.size());
  if (appCount == 0) {
    renderer.drawCenteredText(UI_10_FONT_ID, appsTop + appsHeight / 2, tr(STR_NO_INSTALLED_APPS));
  } else {
    const int appSelected = isMenuIndex(selectedIndex) ? -1 : appIndexFor(selectedIndex);
    GUI.drawList(renderer, Rect{0, appsTop, pageWidth, appsHeight}, appCount, appSelected,
                 [this](int index) { return entries_[static_cast<size_t>(index)].name; },
                 [this](int index) { return entries_[static_cast<size_t>(index)].version; });
  }

  const auto labels = mappedInput.mapLabels(tr(STR_BACK), tr(STR_SELECT), tr(STR_DIR_UP), tr(STR_DIR_DOWN));
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);

  renderer.displayBuffer();
}
