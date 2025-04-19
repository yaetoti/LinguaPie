#pragma once

#include "WindowEvent.hpp"

struct MouseScrollWindowEvent final : WindowEvent {
  MouseScrollWindowEvent(float xCursor, float yCursor, float delta)
  : WindowEvent(WindowEventType::MOUSE_SCROLL)
  , xCursor(xCursor)
  , yCursor(yCursor)
  , delta(delta) {
  }

  float xCursor;
  float yCursor;
  float delta;
};
