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

    m_selectedSegment = (int)(mouseAngle / segmentSize) % m_segments;

    return;
  }
}

LRESULT Application::HandleKeyboardHook(int nCode, WPARAM wParam, LPARAM lParam) {
  KBDLLHOOKSTRUCT* data = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);

  if (wParam == WM_KEYDOWN) {
    if (data->vkCode == VK_LWIN || data->vkCode == VK_RWIN) {
      m_windowsPressed = true;
      return CallNextHookEx(nullptr, nCode, wParam, lParam);
    }

    if (data->vkCode == VK_SPACE && m_windowsPressed) {
      // Handle overlay show
      m_spacePressed = true;
      return -1;
    }
  }

  if (wParam == WM_KEYUP) {
    if (data->vkCode == VK_LWIN || data->vkCode == VK_RWIN) {
      // Handle overlay hide
      m_windowsPressed = false;
      m_spacePressed = false;

      return CallNextHookEx(nullptr, nCode, wParam, lParam);
    }

    if (data->vkCode == VK_SPACE) {
      m_spacePressed = false;
      // Handle overlay hide
      return CallNextHookEx(nullptr, nCode, wParam, lParam);
    }
  }

  return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

void Application::Update() {
  if (!m_isWindowShown) {
    if (m_windowsPressed && m_spacePressed) {
      // Show window
      POINT cursorPos;
      GetCursorPos(&cursorPos);
      HMONITOR monitorHandle = MonitorFromPoint(cursorPos, MONITOR_DEFAULTTONEAREST);

      MONITORINFO monitorInfo;
      monitorInfo.cbSize = sizeof(monitorInfo);
      GetMonitorInfoW(monitorHandle, &monitorInfo);

      RECT monitorRect = monitorInfo.rcMonitor;
      SetWindowPos(
        m_window->GetHandle(),
        HWND_TOPMOST,
        monitorRect.left,
        monitorRect.top,
        monitorRect.right - monitorRect.left,
        monitorRect.bottom - monitorRect.top,
        SWP_SHOWWINDOW
      );

      m_isWindowShown = true;
    }
  }
  else {
    if (!m_windowsPressed || !m_spacePressed) {
      // Hide window
      m_window->Show(SW_HIDE);
      m_isWindowShown = false;
    }
  }

  if (!m_isWindowShown) {
    return;
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
  m_frameBuffer.data.msaaLevel = 2;
  m_frameBuffer.data.segments = m_segments;
  m_frameBuffer.data.activeSegment = m_selectedSegment;
  m_frameBuffer.Init();
}

void Application::Render() const {
  if (!m_isWindowShown) {
    return;
  }

  auto* dc = DxContext::Get()->d3d11Context.Get();
  auto* dc2D = DxContext::Get()->d2d1Context.Get();
  auto* dwriteFactory = DxContext::Get()->dwriteFactory.Get();

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

  // Draw text
  dc2D->SetTarget(m_window->GetBackBuffer2D());

  ComPtr<IDWriteTextFormat> textFormat;
  dwriteFactory->CreateTextFormat(
    L"Roboto",
    nullptr,
    DWRITE_FONT_WEIGHT_MEDIUM,
    DWRITE_FONT_STYLE_NORMAL,
    DWRITE_FONT_STRETCH_NORMAL,
    32,
    L"",
    textFormat.ReleaseAndGetAddressOf()
  );

  ComPtr<ID2D1SolidColorBrush> brush;
  dc2D->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f), brush.ReleaseAndGetAddressOf());

  dc2D->BeginDraw();
  //dc2D->DrawLine(D2D1::Point2F(0, 0), D2D1::Point2F(1920, 1080), brush.Get(), 4);

  std::wstring text[] = {
    L"EN (US)",
    L"UA",
    L"EN (INTL)",
    L"RU",
  };

  D2D1_POINT_2F positions[4] = {
    D2D1::Point2F(1100, 400),
    D2D1::Point2F(1081, 656),
    D2D1::Point2F(810, 690),
    D2D1::Point2F(820, 369),
  };

  D2D1_POINT_2F centers[4];

  DWRITE_TEXT_METRICS metrics[4];
  ComPtr<IDWriteTextLayout> layouts[4];

  for (int i = 0; i < 4; i++) {
    dwriteFactory->CreateTextLayout(
      text[i].c_str(),
      text[i].length(),
      textFormat.Get(),
      1000.0f,
      1000.0f,
      layouts[i].ReleaseAndGetAddressOf()
    );

    layouts[i]->GetMetrics(&metrics[i]);

    centers[i].x = positions[i].x - metrics[i].width / 2;
    centers[i].y = positions[i].y - metrics[i].height / 2;
  }

  D2D1::Matrix3x2F transforms[] = {
    D2D1::Matrix3x2F::Rotation(-45, positions[0]),
    D2D1::Matrix3x2F::Rotation(45, positions[1]),
    D2D1::Matrix3x2F::Rotation(-45, positions[2]),
    D2D1::Matrix3x2F::Rotation(45, positions[3]),
  };

  for (int i = 0; i < 4; i++) {
    dc2D->SetTransform(transforms[i]);
    dc2D->DrawTextLayout(
      centers[i],
      layouts[i].Get(),
      brush.Get(),
      D2D1_DRAW_TEXT_OPTIONS_NONE
    );
  }


  dc2D->EndDraw();

  // Blit
  m_window->Present();
}
