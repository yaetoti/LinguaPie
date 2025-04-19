#include "Application.hpp"

#include <DirectXMath.h>
#include <iostream>
#include <window/OverlayWindow.hpp>

#include "rendering/buffers/ConstantBuffer.hpp"
#include "rendering/buffers/data/FrameData.hpp"
#include "utils/ColorUtils.hpp"

Application::Application()
: m_isRunning(true) {
  m_window = std::make_shared<OverlayWindow>();
}

Application::~Application() {
  Cleanup();
}

bool Application::Initialize() {
  if (!m_window->Initialize()) {
    return false;
  }

  m_menuPipeline.Init(L"Assets/shaders/pie.hlsl", ShaderType::VERTEX_SHADER | ShaderType::PIXEL_SHADER);

  return true;
}

void Application::Cleanup() {
  m_window->Cleanup();
}

void Application::RunMainLoop() {
  // Main loop
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

    Update();
    Render();
  }
}

void Application::Update() {
  static int segment = 0;
  static int timer = 0;

  ++timer;
  if (timer > 120) {
    timer = 0;
    ++segment;
    if (segment >= 4) {
      segment = 0;
    }
  }

  // Set FrameData
  m_frameBuffer.data.resolution = DirectX::XMFLOAT2(
    static_cast<float>(m_window->GetWidth()),
    static_cast<float>(m_window->GetHeight())
  );
  m_frameBuffer.data.darkColor = ColorUtils::RgbFromHex(0x080808);
  m_frameBuffer.data.brightColor = ColorUtils::RgbFromHex(0x1D1D1D);
  m_frameBuffer.data.radius = std::min(m_frameBuffer.data.resolution.x, m_frameBuffer.data.resolution.y) * 0.324f;
  m_frameBuffer.data.innerRadius = std::min(50.0f, 0.15f * m_frameBuffer.data.radius);
  m_frameBuffer.data.msaaLevel = 4;
  m_frameBuffer.data.segments = 4;
  m_frameBuffer.data.activeSegment = segment;
  m_frameBuffer.Init();
}

void Application::Render() const {
  auto* dc = DxContext::Get()->d3d11Context.Get();

  // Set RT
  ID3D11RenderTargetView* views[] = {
    m_window->GetRenderTargetViewMSAA()
  };
  dc->OMSetRenderTargets(1, views, nullptr);

  // Tint
  float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.3f };
  dc->ClearRenderTargetView(m_window->GetRenderTargetViewMSAA(), clearColor);

  // Set resources
  dc->VSSetConstantBuffers(0, 1, m_frameBuffer.GetAddressOf());
  dc->PSSetConstantBuffers(0, 1, m_frameBuffer.GetAddressOf());
  m_menuPipeline.Bind();

  // Draw menu
  dc->Draw(3, 0);

  // Blit
  m_window->Present();
}
