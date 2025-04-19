#include "DomainShader.hpp"
#include "engine/DxContext.hpp"

#include <cassert>
#include <d3dcompiler.h>

ID3DBlob* DomainShader::GetBlob() const {
  return m_blob.Get();
}

ID3D11DomainShader* DomainShader::GetShader() const {
  return m_shader.Get();
}

void DomainShader::Reset() {
  m_shader.Reset();
  m_blob.Reset();
}

void DomainShader::Init(const std::wstring& path) {
  HRESULT result;
  ComPtr<ID3DBlob> errorBlob;

  // Blob
  result = D3DCompileFromFile(
    path.c_str(),
    nullptr,
    D3D_COMPILE_STANDARD_FILE_INCLUDE,
    "DSMain",
    "ds_5_0",
    D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG,
    0,
    m_blob.GetAddressOf(),
    errorBlob.GetAddressOf()
  );
  if (FAILED(result)) {
    printf("%s\n", static_cast<LPCWSTR>(errorBlob->GetBufferPointer()));
    assert(SUCCEEDED(result));
    return;
  }

  // Shader
  result = DxContext::Get()->d3d11Device->CreateDomainShader(
    m_blob->GetBufferPointer(),
    m_blob->GetBufferSize(),
    nullptr,
    m_shader.GetAddressOf()
  );
  assert(SUCCEEDED(result));
  if (FAILED(result)) {
    return;
  }
}