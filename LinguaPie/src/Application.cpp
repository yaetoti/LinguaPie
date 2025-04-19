#include "Application.hpp"

#include <DirectXMath.h>
#include <iostream>
#include <window/OverlayWindow.hpp>

#include "rendering/buffers/ConstantBuffer.hpp"
#include "rendering/buffers/data/FrameData.hpp"
#include "utils/ColorUtils.hpp"
#include "window/events/KeyWindowEvent.hpp"
#include "window/events/MouseMoveWindowEvent.hpp"

Application::Application()
: m_isRunning(true) {
  m_window = std::make_shared<OverlayWindow>();
}

Application::~Application() {
  Cleanup();
}

bool Application::Initialize() {
  m_window->GetDispatcher().AddListener(this);
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

void Application::HandleEvent(const WindowEvent& e) {
  if (e.type == WindowEventType::KEY) {
    auto* event = e.As<KeyWindowEvent>();
    if (event->vkCode == VK_ESCAPE) {
      m_isRunning = false;
    }

    return;
  }

  if (e.type == WindowEventType::MOUSE_MOVE) {
    auto* event = e.As<MouseMoveWindowEvent>();

    std::wcout << L"Mouse moved:" << std::endl;
    std::wcout << L"X: " << event->xCursor << std::endl;
    std::wcout << L"Y: " << event->yCursor << std::endl;
    std::wcout << std::endl;

    // Calculate current segment

    constexpr float PI = 3.14159265358976f;
    float segmentSize = 2 * PI / m_segments;
    float centerX = m_window->GetWidth() / 2;
    float centerY = m_window->GetHeight() / 2;
    float mouseVecX = event->xCursor - centerX;
    float mouseVecY = event->yCursor - centerY;

    float mouseAngle = atan2(mouseVecY, mouseVecX) + PI * 0.5;
    if (mouseAngle < 0.0f) {
      mouseAngle += 2.0f * PI;
    }

    m_mouseSegment = (int)(mouseAngle / segmentSize) % m_segments;

    return;
  }
}

void Application::Update() {
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
  m_frameBuffer.data.segments = m_segments;
  m_frameBuffer.data.activeSegment = m_mouseSegment;
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
