#pragma once

#include <DirectXMath.h>

struct FrameData final {
  float radius;
  float innerRadius;
  int msaaLevel;
  int segments;
  int activeSegment;
  DirectX::XMFLOAT3 darkColor;
  DirectX::XMFLOAT3 brightColor;
  char _pad0[1];
  DirectX::XMFLOAT2 resolution;
  char _pad1[2];
};