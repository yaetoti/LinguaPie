#pragma once
#include <memory>
#include <window/OverlayWindow.hpp>

struct Application final {
  Application();
  ~Application();

  bool Initialize();
  void Cleanup();

  void RunMainLoop();

private:
  bool m_isRunning;
  std::shared_ptr<OverlayWindow> m_window;
};
