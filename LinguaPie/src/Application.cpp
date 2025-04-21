#include "Application.hpp"

#include <DirectXMath.h>
#include <iostream>
#include <window/OverlayWindow.hpp>
#include <chrono>
#include <thread>

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
  constexpr double TARGET_FPS = 144.0;
  constexpr std::chrono::duration<double> FRAME_TIME(1.0 / TARGET_FPS);

  auto frameStart = std::chrono::high_resolution_clock::now();
  while (m_isRunning) {
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
      if (msg.message == WM_QUIT) {
        m_isRunning = false;
        break;
      }

      TranslateMessage(&msg);
      DispatchMessageW(&msg);
    }

    auto frameEnd = std::chrono::high_resolution_clock::now();
    auto frameDuration = frameEnd - frameStart;
    if (frameDuration > FRAME_TIME) {
      frameStart = frameEnd;

      Update();
      Render();
      //std::this_thread::sleep_for(FRAME_TIME - frameDuration);
    }
  }
}

void Application::HandleEvent(const WindowEvent& e, CallbackInfo& info) {
  if (e.type == WindowEventType::KEY) {
    auto* event = e.As<KeyWindowEvent>();
    if (event->vkCode == VK_ESCAPE) {
      m_isRunning = false;
    }

    return;
  }

  if (e.type == WindowEventType::MOUSE_MOVE) {
    auto* event = e.As<MouseMoveWindowEvent>();

    // Calculate current segment

    constexpr float PI = 3.14159265358976f;
    float segmentSize = 2 * PI / m_segments;
    float centerX = m_window->GetWidth() / 2;
    float centerY = m_window->GetHeight() / 2;
    float mouseVecX = event->xCursor - centerX;
    float mouseVecY = event->yCursor - centerY;
    float dist = sqrtf(mouseVecX * mouseVecX + mouseVecY * mouseVecY);
    if (dist < m_innerRadius) {
      m_selectedSegment = m_currentSegment;
      return;
    }

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
      //return -1;
      return CallNextHookEx(nullptr, nCode, wParam, lParam);
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

void Application::HandleEvent(const LLMouseHookEvent &e, CallbackInfo &info) {
  auto* data = reinterpret_cast<MSLLHOOKSTRUCT*>(e.lParam);
  if (e.wParam == WM_XBUTTONDOWN && HIWORD(data->mouseData) == 1) {
    m_xMousePressed = true;

    e.SetReturnValue(-1);
    info.Cancel();
    PostMessageW(m_window->GetHandle(), WM_USER, 0, 0);
    return;
  }

  if (e.wParam == WM_XBUTTONUP && HIWORD(data->mouseData) == 1) {
    m_xMousePressed = false;

    e.SetReturnValue(-1);
    info.Cancel();
    PostMessageW(m_window->GetHandle(), WM_USER, 0, 0);
    return;
  }
}

void Application::Update() {
  // Window needs a one-tick delay to process a language change request
  if (m_isWindowClosing) {
    m_window->Show(SW_HIDE);
    m_isWindowShown = false;
    m_isWindowClosing = false;
  }

  if (!m_isWindowShown) {
    if (m_xMousePressed) {
      // Find display
      POINT cursorPos;
      GetCursorPos(&cursorPos);
      HMONITOR monitorHandle = MonitorFromPoint(cursorPos, MONITOR_DEFAULTTONEAREST);

      // Get display rect
      MONITORINFO monitorInfo;
      monitorInfo.cbSize = sizeof(monitorInfo);
      GetMonitorInfoW(monitorHandle, &monitorInfo);

      RECT monitorRect = monitorInfo.rcMonitor;
      int width = monitorRect.right - monitorRect.left;
      int height = monitorRect.bottom - monitorRect.top;

      // Recalculate values
      m_frameBuffer.data.resolution = DirectX::XMFLOAT2(
        static_cast<float>(width),
        static_cast<float>(height)
      );
      m_radius = std::min(width, height) * 0.324f;
      m_innerRadius = std::min(50.0f, 0.15f * m_radius);

      // Load layouts
      int layoutsCount = GetKeyboardLayoutList(0, nullptr);

      m_layouts.resize(layoutsCount);
      GetKeyboardLayoutList(layoutsCount, m_layouts.data());

      m_segments = layoutsCount;

      // Get active layout
      HWND hwnd = GetForegroundWindow();
      if (!hwnd) {
        hwnd = GetDesktopWindow();
      }

      DWORD threadId = GetWindowThreadProcessId(hwnd, nullptr);
      HKL activeLayout = GetKeyboardLayout(threadId);

      // Find its index in the list
      int activeIndex = 0;
      for (int i = 0; i < layoutsCount; i++) {
        if (m_layouts[i] == activeLayout) {
          activeIndex = i;
          break;
        }
      }

      m_currentSegment = activeIndex;

      // Show window
      SetCursorPos(monitorRect.left + width / 2, monitorRect.top + height / 2);
      SetWindowPos(
        m_window->GetHandle(),
        HWND_TOPMOST,
        monitorRect.left,
        monitorRect.top,
        monitorRect.right - monitorRect.left,
        monitorRect.bottom - monitorRect.top,
        SWP_SHOWWINDOW
      );
      //SetForegroundWindow(m_window->GetHandle());

      m_isWindowShown = true;
    }
  }
  else {
    if (!m_xMousePressed) {
      // Hide a window
      // Needs a one-tick timeout to handle a window message
      SendMessageW(GetForegroundWindow(), WM_INPUTLANGCHANGEREQUEST, 0, LPARAM(m_layouts.at(m_selectedSegment)));
      //SendMessageW(m_window->GetHandle(), WM_INPUTLANGCHANGEREQUEST, 0, LPARAM(m_layouts.at(m_selectedSegment)));
      m_isWindowClosing = true;
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
  m_frameBuffer.data.radius = m_radius;
  m_frameBuffer.data.innerRadius = m_innerRadius;
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

  float textScale = m_radius / 350.0f;
  ComPtr<IDWriteTextFormat> textFormat;
  dwriteFactory->CreateTextFormat(
    L"Roboto",
    nullptr,
    DWRITE_FONT_WEIGHT_MEDIUM,
    DWRITE_FONT_STYLE_NORMAL,
    DWRITE_FONT_STRETCH_NORMAL,
    32 * textScale,
    L"",
    textFormat.ReleaseAndGetAddressOf()
  );

  ComPtr<ID2D1SolidColorBrush> brush;
  dc2D->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f), brush.ReleaseAndGetAddressOf());

  dc2D->BeginDraw();
  //dc2D->DrawLine(D2D1::Point2F(0, 0), D2D1::Point2F(1920, 1080), brush.Get(), 4);


  std::vector<std::wstring> text;
  for (int i = 0; i < m_segments; i++) {
    uint32_t layoutId = (reinterpret_cast<uint32_t>(m_layouts.at(i)) >> 16) & 0xFFFF;
    if (m_layoutMap.contains(layoutId)) {
      text.emplace_back(m_layoutMap.at(layoutId));
    } else {
      text.emplace_back(L"Unknown");
    }
  }
  // std::wstring text[] = {
  //   L"RU",
  //   L"EN (US)",
  //   L"EN (INTL)",
  //   L"UA",
  // };




  constexpr float PI = 3.14159265358976f;
  float segmentSize = 2 * PI / m_segments;
  float centerX = m_window->GetWidth() / 2;
  float centerY = m_window->GetHeight() / 2;
  float labelRadius = m_innerRadius + (m_radius - m_innerRadius) * 0.5f;

  std::vector<D2D1_POINT_2F> positions(m_segments);
  std::vector<D2D1_POINT_2F> centers(m_segments);
  std::vector<DWRITE_TEXT_METRICS> metrics(m_segments);
  std::vector<ComPtr<IDWriteTextLayout>> layouts(m_segments);
  std::vector<D2D1::Matrix3x2F> transforms(m_segments);

  for (int i = 0; i < m_segments; i++) {
    float angle = -segmentSize * (i + 0.5) + PI * 0.5f;
    float rotationAngle = -angle;
    if (rotationAngle > PI * 0.5f + 0.001f) {
      rotationAngle -= PI;
    }

    if (rotationAngle < -PI * 0.5f - 0.001f) {
      rotationAngle += PI;
    }

    positions[i] = D2D1::Point2F(
      centerX + labelRadius * cosf(angle),
      centerY - labelRadius * sinf(angle)
    );

    dwriteFactory->CreateTextLayout(
      text[i].c_str(),
      text[i].length(),
      textFormat.Get(),
      (m_radius - m_innerRadius) * 0.9f,
      32.0f * textScale,
      layouts[i].ReleaseAndGetAddressOf()
    );

    layouts[i]->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
    DWRITE_TRIMMING trimming = { DWRITE_TRIMMING_GRANULARITY_CHARACTER, 0, 0 };
    ComPtr<IDWriteInlineObject> inlineObject;
    dwriteFactory->CreateEllipsisTrimmingSign(textFormat.Get(), inlineObject.ReleaseAndGetAddressOf());
    layouts[i]->SetTrimming(&trimming, inlineObject.Get());

    layouts[i]->GetMetrics(&metrics[i]);

    centers[i].x = positions[i].x - metrics[i].width / 2;
    centers[i].y = positions[i].y - metrics[i].height / 2;

    transforms[i] = D2D1::Matrix3x2F::Rotation(rotationAngle * 180.0f / PI, positions[i]);
  }


  for (int i = 0; i < 4; i++) {
    dc2D->SetTransform(transforms[i]);
    dc2D->DrawTextLayout(
      centers[i],
      layouts[i].Get(),
      brush.Get(),
      D2D1_DRAW_TEXT_OPTIONS_CLIP
    );
  }


  dc2D->EndDraw();

  // Blit
  m_window->Present();
}
