#pragma once
#include <memory>
#include <window/OverlayWindow.hpp>

#include "rendering/buffers/ConstantBuffer.hpp"
#include "rendering/buffers/data/FrameData.hpp"

struct Application final {
  Application();
  ~Application();

  bool Initialize();
  void Cleanup();

  void RunMainLoop();

private:
  void Update();
  void Render() const;

private:
  bool m_isRunning;
  std::shared_ptr<OverlayWindow> m_window;
  ConstantBuffer<FrameData> m_frameBuffer;
  ShaderPipeline m_menuPipeline;
};
