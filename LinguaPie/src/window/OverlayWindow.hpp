#pragma once

#include <wrl/client.h>
#include "engine/DxContext.hpp"

struct OverlayWindow final {
  template<typename T>
  using ComPtr = Microsoft::WRL::ComPtr<T>;

  OverlayWindow();
  ~OverlayWindow();

  bool Initialize();
  void Cleanup();

private:
  HWND m_handle;
  ComPtr<IDXGISwapChain1> m_swapChain;
  ComPtr<ID3D11Texture2D> m_backBuffer;
  ComPtr<ID3D11RenderTargetView> m_bufferView;

  ComPtr<IDCompositionTarget> m_compositionTarget;
  ComPtr<IDCompositionVisual> m_compositionVisual;

  inline static const wchar_t* kClassName = L"OverlayWindowClass";
};
