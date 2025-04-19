#include "Application.hpp"

#include <window/OverlayWindow.hpp>

Application::Application():
m_isRunning(true) {
  m_window = std::make_shared<OverlayWindow>();
}

Application::~Application() {

}

bool Application::Initialize() {
  if (!m_window->Initialize()) {
    return false;
  }

  return true;
}

void Application::Cleanup() {
  m_window->Cleanup();
}


void Application::RunMainLoop() {
  // Main loop
  OverlayWindow overlayWindow;
  overlayWindow.Initialize();

  MSG msg;
  while (m_isRunning) {
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
      if (msg.message == WM_QUIT) {
        m_isRunning = false;
        break;
      }

      TranslateMessage(&msg);
      DispatchMessageW(&msg);
    }
  }
}
