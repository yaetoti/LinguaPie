#pragma once

#include <wrl/client.h>
#include "engine/DxContext.hpp"
#include "rendering/shaders/ShaderPipeline.hpp"

struct OverlayWindow final {
  template<typename T>
  using ComPtr = Microsoft::WRL::ComPtr<T>;

  OverlayWindow();
  ~OverlayWindow();

  bool Initialize();
  void Cleanup();

  void Present() const;

  int GetWidth() const;
  int GetHeight() const;
  ID3D11RenderTargetView* GetRenderTargetViewMSAA() const;

private:
  HWND m_handle;
  int m_width;
  int m_height;

  ComPtr<IDXGISwapChain1> m_swapChain;
  ComPtr<ID3D11Texture2D> m_backBuffer;
  ComPtr<ID3D11Texture2D> m_backBufferMSAA;
  ComPtr<ID3D11RenderTargetView> m_bufferView;
  ComPtr<ID3D11RenderTargetView> m_bufferViewMSAA;
  ComPtr<ID3D11ShaderResourceView> m_bufferShaderViewMSAA;
  ComPtr<ID3D11RasterizerState> m_rasterizerState;
  ComPtr<ID3D11BlendState> m_blendState;

  ComPtr<IDCompositionTarget> m_compositionTarget;
  ComPtr<IDCompositionVisual> m_compositionVisual;

  ShaderPipeline m_resolvePipeline;

  inline static const wchar_t* kClassName = L"OverlayWindowClass";
};
