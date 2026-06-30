#include "AppRunnerActivity.h"

#include <AppPathSanitizer.h>
#include <AppRunner.h>
#include <AppStorePaths.h>
#include <GfxRenderer.h>
#include <I18n.h>
#include <Logging.h>
#include <LuaAppDisplay.h>

#include <cstdio>

#include "activities/util/FullScreenMessageActivity.h"
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
    activityManager.replaceActivity(std::make_unique<FullScreenMessageActivity>(
        renderer, mappedInput, std::string(tr(STR_APP_RUN_FAILED)), EpdFontFamily::REGULAR));
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
    const std::string message = runResult.errorMessage.empty() ? std::string(tr(STR_APP_RUN_FAILED))
                                                               : runResult.errorMessage;
    LOG_ERR("APPS", "App %s failed: %s", appId_.c_str(), message.c_str());
    activityManager.replaceActivity(
        std::make_unique<FullScreenMessageActivity>(renderer, mappedInput, message, EpdFontFamily::REGULAR));
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
  if (state_ != State::Idle) {
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
  } else {
    const auto& draws = LuaAppDisplay::entries();
    if (draws.empty()) {
      renderer.drawCenteredText(UI_12_FONT_ID, pageHeight / 2, tr(STR_APP_PRESS_BACK_TO_EXIT));
    } else {
      // #region agent log
      LOG_DBG("APPS", "AppRunner contentTop=%d entries=%u", contentTop, static_cast<unsigned>(draws.size()));
      // #endregion
      for (const LuaAppDisplayEntry& entry : draws) {
        const int drawY = contentTop + entry.y;
        if (entry.centered) {
          renderer.drawCenteredText(UI_10_FONT_ID, drawY, entry.text.c_str());
        } else {
          renderer.drawText(UI_10_FONT_ID, entry.x, drawY, entry.text.c_str());
        }
      }
      renderer.drawCenteredText(UI_10_FONT_ID, footerY, tr(STR_APP_PRESS_BACK_TO_EXIT));
    }
  }

  const auto labels = mappedInput.mapLabels(tr(STR_BACK), "", "", "");
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);

  renderer.displayBuffer();
}
