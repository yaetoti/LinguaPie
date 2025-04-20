#include "OverlayWindow.hpp"

#include <cassert>
#include <windowsx.h>
#include <DirectXMath.h>
#include <engine/Engine.hpp>
#include <rendering/shaders/ShaderPipeline.hpp>
#include <rendering/buffers/ConstantBuffer.hpp>
#include <rendering/buffers/data/FrameData.hpp>

#include "events/KeyWindowEvent.hpp"
#include "events/MouseButtonWindowEvent.hpp"
#include "events/MouseMoveWindowEvent.hpp"
#include "events/MouseScrollWindowEvent.hpp"
#include "events/ResizeWindowEvent.hpp"
#include "utils/ColorUtils.hpp"

OverlayWindow::OverlayWindow()
: m_handle(nullptr) {
}

OverlayWindow::~OverlayWindow() {
  Cleanup();
}

bool OverlayWindow::Initialize() {
  WNDCLASSEXW wc = { };
  wc.cbSize = sizeof(WNDCLASSEX);
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.style = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = WndProc;
  wc.hInstance = GetModuleHandleW(nullptr);
  wc.lpszClassName = kClassName;
  wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
  wc.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
  wc.hIconSm = LoadIconW(nullptr, IDI_APPLICATION);
  wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
  wc.lpszMenuName = nullptr;
  if (!RegisterClassExW(&wc)) {
    return false;
  }

  // TODO must be created on a certain monitor
  // TODO must be fullscreen

  m_width = 1920;
  m_height = 1080;

  m_handle = CreateWindowExW(
    WS_EX_NOREDIRECTIONBITMAP | WS_EX_TOPMOST,
    kClassName,
    L"LinguaPie",
    WS_POPUP,
    0,
    0,
    m_width,
    m_height,
    nullptr,
    nullptr,
    GetModuleHandle(nullptr),
    this
  );
  if (!m_handle) {
    return false;
  }

  InitHandlers();

  auto* dxgiFactory = DxContext::Get()->dxgiFactory.Get();
  auto* dcompDevice = DxContext::Get()->dcompDevice.Get();
  auto* d2dContext = DxContext::Get()->d2d1Context.Get();
  auto* device = DxContext::Get()->d3d11Device.Get();
  HRESULT status = S_OK;

  // SwapChain
  {
    DXGI_SWAP_CHAIN_DESC1 desc = { };
    desc.Width = m_width;
    desc.Height = m_height;
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.BufferCount = 2;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.BufferUsage = DXGI_USAGE_BACK_BUFFER | DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;
    desc.Scaling = DXGI_SCALING_STRETCH;
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    status = dxgiFactory->CreateSwapChainForComposition(
      device,
      &desc,
      nullptr,
      m_swapChain.ReleaseAndGetAddressOf()
    );
    assert(SUCCEEDED(status));
    if (FAILED(status)) {
      return false;
    }
  }

  // Back Buffer
  {
    status = m_swapChain->GetBuffer(0, IID_PPV_ARGS(m_backBuffer.ReleaseAndGetAddressOf()));
    assert(SUCCEEDED(status));
    if (FAILED(status)) {
      return false;
    }
  }

  // RenderTargetView
  {
    status = DxContext::Get()->d3d11Device->CreateRenderTargetView(
      m_backBuffer.Get(),
      nullptr,
      m_bufferView.ReleaseAndGetAddressOf()
    );
    assert(SUCCEEDED(status));
    if (FAILED(status)) {
      return false;
    }
  }

  // MSAA Back Buffer
  {
    D3D11_TEXTURE2D_DESC desc = { };
    desc.ArraySize = 1;
    desc.MipLevels = 1;
    desc.Width = m_width;
    desc.Height = m_height;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    status = device->CreateTexture2D(&desc, nullptr, m_backBufferMSAA.ReleaseAndGetAddressOf());
    assert(SUCCEEDED(status));
    if (FAILED(status)) {
      return false;
    }
  }

  // MSAA RenderTargetView
  {
    status = DxContext::Get()->d3d11Device->CreateRenderTargetView(
      m_backBufferMSAA.Get(),
      nullptr,
      m_bufferViewMSAA.ReleaseAndGetAddressOf()
    );
    assert(SUCCEEDED(status));
    if (FAILED(status)) {
      return false;
    }
  }

  // MSAA SRV
  {
    D3D11_SHADER_RESOURCE_VIEW_DESC desc { };
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
    desc.Texture2DMS = { };

    status = DxContext::Get()->d3d11Device->CreateShaderResourceView(
     m_backBufferMSAA.Get(),
     &desc,
     m_bufferShaderViewMSAA.ReleaseAndGetAddressOf()
    );
    assert(SUCCEEDED(status));
    if (FAILED(status)) {
      return false;
    }
  }

  // D2D1 back buffer
  {
    ComPtr<IDXGISurface> surface;
    status = m_backBufferMSAA.As(&surface);
    assert(SUCCEEDED(status));
    if (FAILED(status)) {
      return false;
    }

    D2D1_BITMAP_PROPERTIES1 props = D2D1::BitmapProperties1(
      D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
      D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
    );
    
    status = d2dContext->CreateBitmapFromDxgiSurface(
      surface.Get(),
      &props,
      m_d2d1BackBuffer.ReleaseAndGetAddressOf()
    );
    assert(SUCCEEDED(status));
    if (FAILED(status)) {
      return false;
    }
  }

  // Rasterizer State
  {
    D3D11_RASTERIZER_DESC desc = { };
    desc.FillMode = D3D11_FILL_SOLID;
    desc.CullMode = D3D11_CULL_BACK;
    desc.FrontCounterClockwise = FALSE;
    desc.DepthClipEnable = FALSE;
    desc.ScissorEnable = FALSE;
    desc.AntialiasedLineEnable = TRUE;
    desc.MultisampleEnable = TRUE;

    status = device->CreateRasterizerState(&desc, m_rasterizerState.ReleaseAndGetAddressOf());
    assert(SUCCEEDED(status));
    if (FAILED(status)) {
      return false;
    }

    DxContext::Get()->d3d11Context->RSSetState(m_rasterizerState.Get());
  }

  // Blend State
  {
    D3D11_BLEND_DESC desc = { };
    desc.RenderTarget[0].BlendEnable = TRUE;
    desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_MAX;
    desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
    desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    status = device->CreateBlendState(&desc, m_blendState.ReleaseAndGetAddressOf());
    assert(SUCCEEDED(status));
    if (FAILED(status)) {
      return false;
    }

    DxContext::Get()->d3d11Context->OMSetBlendState(m_blendState.Get(), nullptr, 0xFFFFFFFF);
  }

  // Composition
  {
    status = dcompDevice->CreateTargetForHwnd(
      m_handle,
      TRUE,
      m_compositionTarget.ReleaseAndGetAddressOf()
    );
    assert(SUCCEEDED(status));
    if (FAILED(status)) {
      return false;
    }

    status = dcompDevice->CreateVisual(m_compositionVisual.ReleaseAndGetAddressOf());
    assert(SUCCEEDED(status));
    if (FAILED(status)) {
      return false;
    }

    m_compositionVisual->SetContent(m_swapChain.Get());
    m_compositionTarget->SetRoot(m_compositionVisual.Get());
    dcompDevice->Commit();
  }

  // Viewport
  {
    D3D11_VIEWPORT viewport = { };
    viewport.Width = static_cast<float>(m_width);
    viewport.Height = static_cast<float>(m_height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;

    DxContext::Get()->d3d11Context->RSSetViewports(1, &viewport);
  }

  // Shaders
  m_resolvePipeline.Init(L"Assets/shaders/resolve.hlsl", ShaderType::VERTEX_SHADER | ShaderType::PIXEL_SHADER);

  // Show
  // TODO move elsewhere
  UpdateWindow(m_handle);
  ShowWindow(m_handle, SW_SHOW);

  return true;
}

void OverlayWindow::Cleanup() {
  DestroyWindow(m_handle);
  UnregisterClassW(kClassName, GetModuleHandleW(nullptr));
}

void OverlayWindow::Present() const {
  // MSAA blit
  auto* dc = DxContext::Get()->d3d11Context.Get();
  dc->OMSetRenderTargets(1, m_bufferView.GetAddressOf(), nullptr);
  dc->PSSetShaderResources(0, 1, m_bufferShaderViewMSAA.GetAddressOf());
  m_resolvePipeline.Bind();

  dc->Draw(3, 0);

  // Present
  m_swapChain->Present(1, 0);
}

int OverlayWindow::GetWidth() const {
  return m_width;
}

int OverlayWindow::GetHeight() const {
  return m_height;
}

EventDispatcher<WindowEvent>& OverlayWindow::GetDispatcher() {
  return m_dispatcher;
}

ID3D11RenderTargetView* OverlayWindow::GetRenderTargetViewMSAA() const {
  return m_bufferViewMSAA.Get();
}

ID2D1Bitmap1* OverlayWindow::GetBackBuffer2D() const {
  return m_d2d1BackBuffer.Get();
}

bool OverlayWindow::HandleWindowMessage(UINT msg, WPARAM wParam, LPARAM lParam) const {
  const auto& it = m_messageHandlers.find(msg);
  if (it == m_messageHandlers.end()) {
    return false;
  }

  it->second(msg, wParam, lParam);
  return true;
}

void OverlayWindow::InitHandlers() {
  m_messageHandlers[WM_SIZE] = [this](UINT msg, WPARAM wParam, LPARAM lParam) {
    HandleResizeMessage(msg, wParam, lParam);
  };

  m_messageHandlers[WM_KEYDOWN]
  = m_messageHandlers[WM_KEYUP]
  = m_messageHandlers[WM_SYSKEYDOWN]
  = m_messageHandlers[WM_SYSKEYUP]
  = [this](UINT msg, WPARAM wParam, LPARAM lParam) {
      HandleKeyMessage(msg, wParam, lParam);
    };

  m_messageHandlers[WM_LBUTTONDOWN]
  = m_messageHandlers[WM_LBUTTONUP]
  = m_messageHandlers[WM_RBUTTONDOWN]
  = m_messageHandlers[WM_RBUTTONUP]
  = m_messageHandlers[WM_MBUTTONDOWN]
  = m_messageHandlers[WM_MBUTTONUP]
  = m_messageHandlers[WM_XBUTTONDOWN]
  = m_messageHandlers[WM_XBUTTONUP]
  = [this](UINT msg, WPARAM wParam, LPARAM lParam) {
      HandleMouseButtonMessage(msg, wParam, lParam);
    };

  m_messageHandlers[WM_MOUSEMOVE] = [this](UINT msg, WPARAM wParam, LPARAM lParam) {
    HandleMouseMoveMessage(msg, wParam, lParam);
  };

  m_messageHandlers[WM_MOUSEWHEEL] = [this](UINT msg, WPARAM wParam, LPARAM lParam) {
    HandleMouseScrollMessage(msg, wParam, lParam);
  };
}

void OverlayWindow::HandleResizeMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
  m_width = LOWORD(lParam);
  m_height = HIWORD(lParam);
  m_dispatcher.Dispatch(ResizeWindowEvent(m_width, m_height));
}

void OverlayWindow::HandleKeyMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
  WORD vkCode = LOWORD(wParam);
  WORD keyFlags = HIWORD(lParam);
  BOOL wasPressed = (keyFlags & KF_REPEAT) == KF_REPEAT;
  BOOL isReleased = (keyFlags & KF_UP) == KF_UP;
  m_dispatcher.Dispatch(KeyWindowEvent(vkCode, !isReleased, wasPressed));
}

void OverlayWindow::HandleMouseButtonMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
  float xCursor = static_cast<float>(GET_X_LPARAM(lParam));
  float yCursor = static_cast<float>(GET_Y_LPARAM(lParam));
  switch (msg) {
    case WM_LBUTTONDOWN:
      m_dispatcher.Dispatch(MouseButtonWindowEvent(MouseButton::LEFT, true, xCursor, yCursor));
      return;
    case WM_RBUTTONDOWN:
      m_dispatcher.Dispatch(MouseButtonWindowEvent(MouseButton::RIGHT, true, xCursor, yCursor));
      return;
    case WM_MBUTTONDOWN:
      m_dispatcher.Dispatch(MouseButtonWindowEvent(MouseButton::MIDDLE, true, xCursor, yCursor));
      return;
    case WM_XBUTTONDOWN: {
      MouseButton xButtonType = HIWORD(wParam) == XBUTTON1 ? MouseButton::X1 : MouseButton::X2;
      m_dispatcher.Dispatch(MouseButtonWindowEvent(xButtonType, true, xCursor, yCursor));
      return;
    }
    case WM_LBUTTONUP:
      m_dispatcher.Dispatch(MouseButtonWindowEvent(MouseButton::LEFT, false, xCursor, yCursor));
      return;
    case WM_RBUTTONUP:
      m_dispatcher.Dispatch(MouseButtonWindowEvent(MouseButton::RIGHT, false, xCursor, yCursor));
      return;
    case WM_MBUTTONUP:
      m_dispatcher.Dispatch(MouseButtonWindowEvent(MouseButton::MIDDLE, false, xCursor, yCursor));
      return;
    case WM_XBUTTONUP: {
      MouseButton xButtonType = HIWORD(wParam) == XBUTTON1 ? MouseButton::X1 : MouseButton::X2;
      m_dispatcher.Dispatch(MouseButtonWindowEvent(xButtonType, false, xCursor, yCursor));
      return;
    }
    default:
      break;
  }
}

void OverlayWindow::HandleMouseMoveMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
  float xCursor = static_cast<float>(GET_X_LPARAM(lParam));
  float yCursor = static_cast<float>(GET_Y_LPARAM(lParam));
  m_dispatcher.Dispatch(MouseMoveWindowEvent(xCursor, yCursor));
}

void OverlayWindow::HandleMouseScrollMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
  float xCursor = static_cast<float>(GET_X_LPARAM(lParam));
  float yCursor = static_cast<float>(GET_Y_LPARAM(lParam));
  float delta = static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam)) / WHEEL_DELTA;
  m_dispatcher.Dispatch(MouseScrollWindowEvent(xCursor, yCursor, delta));
}

LRESULT OverlayWindow::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  if (msg == WM_CREATE) {
    CREATESTRUCTW* data = reinterpret_cast<CREATESTRUCTW*>(lParam);
    SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(data->lpCreateParams));
    return 0;
  }

  if (msg == WM_DESTROY) {
    PostQuitMessage(0);
    return 0;
  }

  OverlayWindow* window = reinterpret_cast<OverlayWindow*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
  if (window != nullptr && window->HandleWindowMessage(msg, wParam, lParam)) {
    return 0;
  }
  return DefWindowProcW(hWnd, msg, wParam, lParam);
}
