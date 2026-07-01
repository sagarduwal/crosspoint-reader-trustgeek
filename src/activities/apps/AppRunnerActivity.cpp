#include "AppRunnerActivity.h"

#include <AppPathSanitizer.h>
#include <AppRunner.h>
#include <AppStorePaths.h>
#include <GfxRenderer.h>
#include <I18n.h>
#include <Logging.h>
#include <LuaAppDisplay.h>

#include <cstdio>

#include "components/UITheme.h"
#include "fontIds.h"

AppRunnerActivity::AppRunnerActivity(GfxRenderer& renderer, MappedInputManager& mappedInput, std::string appId,
                                     std::string displayName)
    : Activity("AppRunner", renderer, mappedInput),
      appId_(std::move(appId)),
      displayName_(displayName.empty() ? appId_ : std::move(displayName)) {}

void AppRunnerActivity::onEnter() {
  Activity::onEnter();
  requestUpdateAndWait();

  if (!AppPathSanitizer::isValidAppId(appId_)) {
    errorMessage_ = tr(STR_APP_RUN_FAILED);
    state_ = State::Error;
    requestUpdate();
    return;
  }

  char mainLuaPath[96];
  snprintf(mainLuaPath, sizeof(mainLuaPath), "%s/%s/main.lua", AppStorePaths::kAppsRoot, appId_.c_str());

  AppRunContext runContext;
  runContext.renderer = &renderer;
  runContext.mappedInput = &mappedInput;
  runContext.appId = appId_;
  runContext.displayName = displayName_;
  runContext.fontId = UI_10_FONT_ID;

  const auto& metrics = UITheme::getInstance().getMetrics();
  const int pageHeight = renderer.getScreenHeight();
  const int contentTop = metrics.topPadding + metrics.headerHeight + metrics.verticalSpacing;
  const int footerReserve =
      metrics.buttonHintsHeight + metrics.verticalSpacing + renderer.getLineHeight(UI_10_FONT_ID);
  runContext.contentTop = contentTop;
  runContext.contentHeight = pageHeight - contentTop - footerReserve;

  const AppRunResult runResult = AppRunner::runMainLua(mainLuaPath, runContext);
  if (!runResult.success) {
    errorMessage_ = runResult.errorMessage.empty() ? std::string(tr(STR_APP_RUN_FAILED)) : runResult.errorMessage;
    state_ = State::Error;
    LOG_ERR("APPS", "App %s failed: %s", appId_.c_str(), errorMessage_.c_str());
    requestUpdate();
    return;
  }

  if (runResult.exitRequested) {
    finish();
    return;
  }

  state_ = State::Idle;
  requestUpdate();
}

void AppRunnerActivity::loop() {
  if (state_ != State::Idle && state_ != State::Error) {
    return;
  }

  if (mappedInput.wasPressed(MappedInputManager::Button::Back)) {
    finish();
  }
}

void AppRunnerActivity::render(RenderLock&&) {
  renderer.clearScreen();

  const auto& metrics = UITheme::getInstance().getMetrics();
  const auto pageWidth = renderer.getScreenWidth();
  const auto pageHeight = renderer.getScreenHeight();

  GUI.drawHeader(renderer, Rect{0, metrics.topPadding, pageWidth, metrics.headerHeight}, displayName_.c_str());

  const int contentTop = metrics.topPadding + metrics.headerHeight + metrics.verticalSpacing;
  const int footerY = pageHeight - metrics.buttonHintsHeight - metrics.verticalSpacing - renderer.getLineHeight(UI_10_FONT_ID);

  if (state_ == State::Loading) {
    renderer.drawCenteredText(UI_12_FONT_ID, pageHeight / 2, tr(STR_LOADING));
  } else if (state_ == State::Error) {
    const int contentWidth = pageWidth - 2 * metrics.contentSidePadding;
    const int lineHeight = renderer.getLineHeight(UI_10_FONT_ID);
    const auto lines = renderer.wrappedText(UI_10_FONT_ID, errorMessage_.c_str(), contentWidth, 8);
    int y = contentTop;
    if (lines.empty()) {
      renderer.drawCenteredText(UI_10_FONT_ID, pageHeight / 2, errorMessage_.c_str());
    } else {
      for (const auto& line : lines) {
        renderer.drawText(UI_10_FONT_ID, metrics.contentSidePadding, y, line.c_str());
        y += lineHeight;
      }
    }
    renderer.drawCenteredText(UI_10_FONT_ID, footerY, tr(STR_APP_PRESS_BACK_TO_EXIT));
  } else {
    const auto& draws = LuaAppDisplay::entries();
    const auto& bitmaps = LuaAppDisplay::bitmapEntries();
    if (draws.empty() && bitmaps.empty()) {
      renderer.drawCenteredText(UI_12_FONT_ID, pageHeight / 2, tr(STR_APP_PRESS_BACK_TO_EXIT));
    } else {
      LuaAppDisplay::paint(renderer, UI_10_FONT_ID, appId_, contentTop);
      renderer.drawCenteredText(UI_10_FONT_ID, footerY, tr(STR_APP_PRESS_BACK_TO_EXIT));
    }
  }

  const auto labels = mappedInput.mapLabels(tr(STR_BACK), "", "", "");
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);

  renderer.displayBuffer();
}
