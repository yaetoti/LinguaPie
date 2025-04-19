#include "DxContext.hpp"

#include <cassert>

bool DxContext::Initialize() {
  HRESULT status = S_OK;

  // DXGI Factory
  {
    status = CreateDXGIFactory2(0, IID_PPV_ARGS(dxgiFactory.ReleaseAndGetAddressOf()));
    assert(SUCCEEDED(status));
    if (FAILED(status)) {
      return false;
    }
  }

  // DXGI Adapter
  {
    status = dxgiFactory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(dxgiAdapter.ReleaseAndGetAddressOf()));
    assert(SUCCEEDED(status));
    if (FAILED(status)) {
      return false;
    }
  }

  // Device, DeviceContext, Debug, DXGI Device
  {
    D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };

    status = D3D11CreateDevice(
      dxgiAdapter.Get(),
      D3D_DRIVER_TYPE_UNKNOWN,
      nullptr,
      D3D11_CREATE_DEVICE_SINGLETHREADED | D3D11_CREATE_DEVICE_DEBUG | D3D11_CREATE_DEVICE_BGRA_SUPPORT,
      featureLevels,
      ARRAYSIZE(featureLevels),
      D3D11_SDK_VERSION,
      d3d11Device.ReleaseAndGetAddressOf(),
      nullptr,
      d3d11Context.ReleaseAndGetAddressOf()
    );

    assert(SUCCEEDED(status));
    if (FAILED(status)) {
      return false;
    }

    status = d3d11Device.As(&d3d11Debug);
    assert(SUCCEEDED(status));
    if (FAILED(status)) {
      return false;
    }

    status = d3d11Device.As(&dxgiDevice);
    assert(SUCCEEDED(status));
    if (FAILED(status)) {
      return false;
    }
  }

  // Composition Device
  {
    status = DCompositionCreateDevice3(d3d11Device.Get(), IID_PPV_ARGS(dcompDevice.ReleaseAndGetAddressOf()));
    assert(SUCCEEDED(status));
    if (FAILED(status)) {
      return false;
    }

    status = dcompDevice.As(&dcompDevice3);
    assert(SUCCEEDED(status));
    if (FAILED(status)) {
      return false;
    }
  }

  // DWrite Factory
  status = DWriteCreateFactory(
    DWRITE_FACTORY_TYPE_SHARED,
    __uuidof(IDWriteFactory),
    reinterpret_cast<IUnknown**>(dwriteFactory.ReleaseAndGetAddressOf())
  );
  assert(SUCCEEDED(status));
  if (FAILED(status)) {
    return false;
  }

  return true;
}

void DxContext::Cleanup() {
  dxgiAdapter.Reset();
  dxgiFactory.Reset();

  d3d11Context->ClearState();
  d3d11Context->Flush();
  d3d11Context.Reset();

  dcompDevice.Reset();
  dxgiDevice.Reset();
  d3d11Device.Reset();

  d3d11Debug->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY | D3D11_RLDO_DETAIL);
  d3d11Debug.Reset();
}
