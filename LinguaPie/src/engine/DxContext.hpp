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

  // D2D1
  ComPtr<ID2D1Factory1> d2d1Factory;
  ComPtr<ID2D1Device> d2d1Device;
  ComPtr<ID2D1DeviceContext> d2d1Context;
};
