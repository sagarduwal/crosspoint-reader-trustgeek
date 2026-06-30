#include "DiscoverAppsActivity.h"

#include <AppStoreManifest.h>
#include <GfxRenderer.h>
#include <I18n.h>

#include "ApplicationsMenuActivity.h"
#include "MappedInputManager.h"
#include "components/UITheme.h"
#include "fontIds.h"

DiscoverAppsActivity::DiscoverAppsActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
    : Activity("DiscoverApps", renderer, mappedInput) {}

void DiscoverAppsActivity::onEnter() {
  Activity::onEnter();
  APP_STORE_CATALOG.load();
  entries_ = APP_STORE_CATALOG.getEntries();
  selectedIndex = 0;
  requestUpdate();
}

void DiscoverAppsActivity::loop() {
  if (mappedInput.wasPressed(MappedInputManager::Button::Back)) {
    activityManager.replaceActivity(std::make_unique<ApplicationsMenuActivity>(renderer, mappedInput));
    return;
  }

  if (entries_.empty()) {
    return;
  }

  const int itemCount = static_cast<int>(entries_.size());
  buttonNavigator.onNextRelease([this, itemCount] {
    selectedIndex = ButtonNavigator::nextIndex(selectedIndex, itemCount);
    requestUpdate();
  });

  buttonNavigator.onPreviousRelease([this, itemCount] {
    selectedIndex = ButtonNavigator::previousIndex(selectedIndex, itemCount);
    requestUpdate();
  });
}

void DiscoverAppsActivity::render(RenderLock&&) {
  renderer.clearScreen();

  const auto& metrics = UITheme::getInstance().getMetrics();
  const auto pageWidth = renderer.getScreenWidth();
  const auto pageHeight = renderer.getScreenHeight();

  GUI.drawHeader(renderer, Rect{0, metrics.topPadding, pageWidth, metrics.headerHeight}, tr(STR_DISCOVER_APPS));

  const int contentTop = metrics.topPadding + metrics.headerHeight + metrics.verticalSpacing;
  const int contentHeight = pageHeight - contentTop - metrics.buttonHintsHeight - metrics.verticalSpacing * 2;
  const int itemCount = static_cast<int>(entries_.size());

  if (itemCount == 0) {
    renderer.drawCenteredText(UI_10_FONT_ID, pageHeight / 2, tr(STR_NO_DISCOVER_APPS));
  } else {
    GUI.drawList(renderer, Rect{0, contentTop, pageWidth, contentHeight}, itemCount, selectedIndex,
                 [this](int index) { return entries_[static_cast<size_t>(index)].name; },
                 [this](int index) { return entries_[static_cast<size_t>(index)].version; });
  }

  const auto labels = mappedInput.mapLabels(tr(STR_BACK), "", tr(STR_DIR_UP), tr(STR_DIR_DOWN));
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);

  renderer.displayBuffer();
}
