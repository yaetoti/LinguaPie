#pragma once

#include <DirectXMath.h>

struct FrameData final {
  float radius;
  DirectX::XMFLOAT2 resolution;
  char _pad0[1];
  DirectX::XMFLOAT3 darkColor;
  char _pad1[1];
  DirectX::XMFLOAT3 brightColor;
  char _pad2[1];
};