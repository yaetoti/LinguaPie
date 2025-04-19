#pragma once
#include <memory>
#include <window/OverlayWindow.hpp>

#include "rendering/buffers/ConstantBuffer.hpp"
#include "rendering/buffers/data/FrameData.hpp"

struct Application final : EventListener<WindowEvent> {
  Application();
  ~Application();

  bool Initialize();
  void Cleanup();

  void RunMainLoop();

  void HandleEvent(const WindowEvent& e) override;

private:
  void Update();
  void Render() const;

private:
  bool m_isRunning;
  int m_segments = 5;
  int m_mouseSegment = 0;
  std::shared_ptr<OverlayWindow> m_window;
  ConstantBuffer<FrameData> m_frameBuffer;
  ShaderPipeline m_menuPipeline;
};
