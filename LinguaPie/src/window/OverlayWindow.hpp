#pragma once

#include <functional>
#include <unordered_map>
#include <wrl/client.h>
#include "engine/DxContext.hpp"
#include "events/WindowEvent.hpp"
#include "rendering/shaders/ShaderPipeline.hpp"
#include "utils/EventDispatcher.hpp"

struct OverlayWindow final {
  template<typename T>
  using ComPtr = Microsoft::WRL::ComPtr<T>;

  OverlayWindow();
  ~OverlayWindow();

  bool Initialize();
  void Cleanup();

  void Present() const;
  void Show(int code) const;
  bool HandleWindowMessage(UINT msg, WPARAM wParam, LPARAM lParam) const;

  int GetWidth() const;
  int GetHeight() const;
  EventDispatcher<WindowEvent>& GetDispatcher();
  ID3D11RenderTargetView* GetRenderTargetViewMSAA() const;
  ID2D1Bitmap1* GetBackBuffer2D() const;

private:
  void InitHandlers();
  void HandleResizeMessage(UINT msg, WPARAM wParam, LPARAM lParam);
  void HandleKeyMessage(UINT msg, WPARAM wParam, LPARAM lParam);
  void HandleMouseButtonMessage(UINT msg, WPARAM wParam, LPARAM lParam);
  void HandleMouseMoveMessage(UINT msg, WPARAM wParam, LPARAM lParam);
  void HandleMouseScrollMessage(UINT msg, WPARAM wParam, LPARAM lParam);

  static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
  HWND m_handle;
  int m_width;
  int m_height;
  EventDispatcher<WindowEvent> m_dispatcher;
  std::unordered_map<UINT, std::function<void(UINT, WPARAM, LPARAM)>> m_messageHandlers;

  ComPtr<IDXGISwapChain1> m_swapChain;
  ComPtr<IDXGISurface> m_surface;

  ComPtr<ID3D11Texture2D> m_backBuffer;
  ComPtr<ID3D11Texture2D> m_backBufferMSAA;
  ComPtr<ID3D11RenderTargetView> m_bufferView;
  ComPtr<ID3D11RenderTargetView> m_bufferViewMSAA;
  ComPtr<ID3D11ShaderResourceView> m_bufferShaderViewMSAA;
  ComPtr<ID3D11RasterizerState> m_rasterizerState;
  ComPtr<ID3D11BlendState> m_blendState;

  ComPtr<IDCompositionTarget> m_compositionTarget;
  ComPtr<IDCompositionVisual> m_compositionVisual;

  ComPtr<ID2D1Bitmap1> m_d2d1BackBuffer;

  ShaderPipeline m_resolvePipeline;

  inline static const wchar_t* kClassName = L"OverlayWindowClass";
};
