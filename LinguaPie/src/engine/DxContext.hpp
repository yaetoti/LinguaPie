#pragma once

#include <Windows.h>
#include <wrl/client.h>
#include <dxgi1_6.h>
#include <d3d11.h>
#include <dcomp.h>
#include <dwrite.h>

struct DxContext final {
  template<typename T>
  using ComPtr = Microsoft::WRL::ComPtr<T>;

  bool Initialize();
  void Cleanup();

  static DxContext* Get() {
    if (m_instance == nullptr) {
      m_instance = new DxContext();
    }

    return m_instance;
  }

  static void Destroy() {
    if (m_instance != nullptr) {
      m_instance->Cleanup();

      delete m_instance;
      m_instance = nullptr;
    }
  }

public:
  inline static DxContext* m_instance = nullptr;

  // DXGI
  ComPtr<IDXGIDevice> dxgiDevice;
  ComPtr<IDXGIAdapter> dxgiAdapter;
  ComPtr<IDXGIFactory6> dxgiFactory;

  // D3D11
  ComPtr<ID3D11Debug> d3d11Debug;
  ComPtr<ID3D11Device> d3d11Device;
  ComPtr<ID3D11DeviceContext> d3d11Context;

  // DirectComposition
  ComPtr<IDCompositionDevice> dcompDevice;
  ComPtr<IDCompositionDevice3> dcompDevice3;

  // DirectWrite
  ComPtr<IDWriteFactory> dwriteFactory;

  ComPtr<IDXGISwapChain> swapChain;
  ComPtr<ID3D11RenderTargetView> renderTargetView;
  ComPtr<ID3D11DepthStencilView> depthStencilView;
  ComPtr<ID3D11VertexShader> vertexShader;
  ComPtr<ID3D11PixelShader> pixelShader;
  ComPtr<ID3D11InputLayout> inputLayout;
  ComPtr<ID3D11Buffer> vertexBuffer;
  ComPtr<ID3D11Buffer> indexBuffer;
  ComPtr<ID3D11Buffer> constantBuffer;
  ComPtr<ID3D11SamplerState> sampler;
  ComPtr<ID3D11RasterizerState> rasterizer;
  ComPtr<ID3D11BlendState> blend;
  ComPtr<ID3D11DepthStencilState> depthStencil;
};
