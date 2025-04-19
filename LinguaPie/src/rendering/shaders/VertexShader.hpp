#pragma once
#include <d3d11.h>
#include <string>
#include <wrl/client.h>

struct VertexShader final {
  template <typename T>
  using ComPtr = Microsoft::WRL::ComPtr<T>;

  [[nodiscard]] ID3DBlob* GetBlob() const;
  [[nodiscard]] ID3D11VertexShader* GetShader() const;

  void Reset();
  void Init(const std::wstring& path);

private:
  ComPtr<ID3DBlob> m_blob;
  ComPtr<ID3D11VertexShader> m_shader;
};
