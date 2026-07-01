#include "FullScreenMessageActivity.h"

#include <GfxRenderer.h>
#include <I18n.h>

#include "components/UITheme.h"
#include "fontIds.h"

void FullScreenMessageActivity::onEnter() {
  Activity::onEnter();
  requestUpdate();
}

void FullScreenMessageActivity::loop() {
  if (mappedInput.wasPressed(MappedInputManager::Button::Back)) {
    finish();
  }
}

void FullScreenMessageActivity::render(RenderLock&&) {
  renderer.clearScreen();

  const auto& metrics = UITheme::getInstance().getMetrics();
  const auto pageWidth = renderer.getScreenWidth();
  const auto pageHeight = renderer.getScreenHeight();
  const auto lineHeight = renderer.getLineHeight(UI_10_FONT_ID);
  const int contentWidth = pageWidth - 2 * metrics.contentSidePadding;

  const auto lines = renderer.wrappedText(UI_10_FONT_ID, text.c_str(), contentWidth, 10);
  int totalHeight = static_cast<int>(lines.size()) * lineHeight;
  int y = (pageHeight - totalHeight) / 2;
  if (y < metrics.topPadding + metrics.headerHeight) {
    y = metrics.topPadding + metrics.headerHeight + metrics.verticalSpacing;
  }
  for (const auto& line : lines) {
    renderer.drawText(UI_10_FONT_ID, metrics.contentSidePadding, y, line.c_str());
    y += lineHeight;
  }

  const auto labels = mappedInput.mapLabels(tr(STR_BACK), "", "", "");
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);

  renderer.displayBuffer(refreshMode);
}
