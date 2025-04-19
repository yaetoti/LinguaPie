#pragma once
#include <DirectXMath.h>

struct ColorUtils final {
  static DirectX::XMFLOAT3 RgbFromHex(uint32_t code) {
    return {
      static_cast<float>((code >> 16) & 0xFF) / 255.0f,
      static_cast<float>((code >> 8) & 0xFF) / 255.0f,
      static_cast<float>((code >> 0) & 0xFF) / 255.0f
    };
  }

  static DirectX::XMFLOAT4 RgbaFromHex(uint32_t code) {
    return {
      static_cast<float>((code >> 24) & 0xFF) / 255.0f,
      static_cast<float>((code >> 16) & 0xFF) / 255.0f,
      static_cast<float>((code >> 8) & 0xFF) / 255.0f,
      static_cast<float>((code >> 0) & 0xFF) / 255.0f
    };
  }
};
