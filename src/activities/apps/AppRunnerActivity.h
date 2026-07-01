#pragma once

#include <string>

#include "activities/Activity.h"

class AppRunnerActivity final : public Activity {
  enum class State { Loading, Idle, Error };

  std::string appId_;
  std::string displayName_;
  std::string errorMessage_;
  State state_ = State::Loading;

 public:
  AppRunnerActivity(GfxRenderer& renderer, MappedInputManager& mappedInput, std::string appId,
                    std::string displayName);

  void onEnter() override;
  void loop() override;
  void render(RenderLock&&) override;
};
