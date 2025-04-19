#include "OverlayWindow.hpp"

#include <cassert>
#include <DirectXMath.h>

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
  wc.lpfnWndProc = DefWindowProc;
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
  // TODO must be topmost (?)

  m_handle = CreateWindowExW(
    WS_EX_NOREDIRECTIONBITMAP,
    kClassName,
    L"LinguaPie",
    WS_POPUP,
    CW_USEDEFAULT,
    CW_USEDEFAULT,
    800,
    600,
    nullptr,
    nullptr,
    GetModuleHandle(nullptr),
    this
  );
  if (!m_handle) {
    return false;
  }

  auto* dxgiFactory = DxContext::Get()->dxgiFactory.Get();
  auto* dcompDevice = DxContext::Get()->dcompDevice.Get();
  auto* device = DxContext::Get()->d3d11Device.Get();
  HRESULT status = S_OK;

  // SwapChain
  {
    DXGI_SWAP_CHAIN_DESC1 desc = { };
    desc.Width = 800;
    desc.Height = 600;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
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

  // TODO move elsewhere
  UpdateWindow(m_handle);
  ShowWindow(m_handle, SW_SHOW);

  return true;
}

void OverlayWindow::Cleanup() {
  DestroyWindow(m_handle);
  UnregisterClassW(kClassName, GetModuleHandleW(nullptr));
}
