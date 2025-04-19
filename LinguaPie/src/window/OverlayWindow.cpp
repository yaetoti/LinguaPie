#include "OverlayWindow.hpp"

#include <cassert>
#include <DirectXMath.h>
#include <engine/Engine.hpp>
#include <rendering/shaders/ShaderPipeline.hpp>
#include <rendering/buffers/ConstantBuffer.hpp>
#include <rendering/buffers/data/FrameData.hpp>

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

  int width = 1920;
  int height = 1080;

  m_handle = CreateWindowExW(
    WS_EX_NOREDIRECTIONBITMAP | WS_EX_TOPMOST,
    kClassName,
    L"LinguaPie",
    WS_POPUP,
    0,
    0,
    width,
    height,
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
    desc.Width = width;
    desc.Height = height;
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

  // Viewport
  {
    D3D11_VIEWPORT viewport = { };
    viewport.Width = static_cast<float>(width);
    viewport.Height = static_cast<float>(height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;

    DxContext::Get()->d3d11Context->RSSetViewports(1, &viewport);
  }

  // TODO move elsewhere
  UpdateWindow(m_handle);
  ShowWindow(m_handle, SW_SHOW);

  {
    auto* dc = DxContext::Get()->d3d11Context.Get();
    dc->OMSetRenderTargets(1, m_bufferView.GetAddressOf(), nullptr);

    // 0 0 0 0.3 - tint
    // 0.03125 - darker
    // 0.11328125 - lighter

    float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.3f };
    dc->ClearRenderTargetView(m_bufferView.Get(), clearColor);

    ConstantBuffer<FrameData> buffer;
    buffer.data.resolution.x = static_cast<float>(width);
    buffer.data.resolution.y = static_cast<float>(height);
    buffer.Init();
    dc->VSSetConstantBuffers(0, 1, buffer.GetAddressOf());
    dc->PSSetConstantBuffers(0, 1, buffer.GetAddressOf());

    ShaderPipeline pipeline;
    pipeline.Init(L"Assets/shaders/pie.hlsl", ShaderType::VERTEX_SHADER | ShaderType::PIXEL_SHADER);
    pipeline.Bind();

    dc->Draw(3, 0);


    m_swapChain->Present(1, 0);
  }

  return true;
}

void OverlayWindow::Cleanup() {
  DestroyWindow(m_handle);
  UnregisterClassW(kClassName, GetModuleHandleW(nullptr));
}
