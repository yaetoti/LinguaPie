#define PI 3.14159265358976f

cbuffer FrameData : register(b0)
{
  float g_radius;
  float g_innerRadius;
  int g_msaaLevel;
  int g_segments;
  int g_activeSegment;
  float3 g_darkColor;
  float3 g_brightColor;
  float2 g_resolution;
};

// Vertex

struct VSInput {
  uint vertexId : SV_VertexID;
};

struct VSOutput {
  float4 position : SV_POSITION;
};

VSOutput VSMain(VSInput input) {
  VSOutput output;

  float2 center = g_resolution / 2;

  // Bottom-Left
  if (input.vertexId == 2) {
    output.position = float4(
      ((center.x - g_radius) / g_resolution.x) * 2 - 1,
      ((center.y - g_radius) / g_resolution.y) * 2 - 1,
      0.0f,
      1.0f
    );
    //output.position = float4(-1.0f, -1.0f, 0.0f, 1.0f);
  }

  // Bottom-Right
  if (input.vertexId == 1) {
    output.position = float4(
      ((center.x + g_radius * 3) / g_resolution.x) * 2 - 1,
      ((center.y - g_radius) / g_resolution.y) * 2 - 1,
      0.0f,
      1.0f
    );
    //output.position = float4(3.0f, -1.0f, 0.0f, 1.0f);
  }

  // Top-Left
  if (input.vertexId == 0) {
    output.position = float4(
      ((center.x - g_radius) / g_resolution.x) * 2 - 1,
      ((center.y + g_radius * 3) / g_resolution.y) * 2 - 1,
      0.0f,
      1.0f
    );
    //output.position = float4(-1.0f, 3.0f, 0.0f, 1.0f);
  }

  return output;
}

// Pixel

struct PSOutput {
  float4 color : SV_TARGET;
};

float4 CalculateColor(double2 position) {
  double2 center = g_resolution / 2;
  double2 pixelVec = position - center;
  double distance = length(pixelVec);

  // Out of bounds - discard
  if (distance > g_radius) {
    return float4(0.0, 0.0, 0.0, 0.0);
  }

  // Inner circle
  if (distance < g_innerRadius) {
    return float4(g_brightColor, 1.0);
  }

  // Calculate current segment
  float segmentSize = 2 * PI / g_segments;

  float pixelAngle = atan2(pixelVec.y, pixelVec.x) + PI * 0.5;
  if (pixelAngle < 0.0f) {
    pixelAngle += 2.0f * PI;
  }

  int pixelSegment = (int)(pixelAngle / segmentSize) % g_segments;
  if (pixelSegment == g_activeSegment) {
    return float4(g_brightColor, 1.0);
  }

  // Outer circle
  return float4(g_darkColor, 1.0);
}

float4 CalculateColorMSAA(int2 position, int resolution) {
  // Grid MSAA
  double2 delta = double2(1, 1) / double2(g_resolution);
  double2 msaaDelta = (double2(1, 1) / resolution);
  //msaaDelta = 4.0.xx;
  double2 msaaOffset = msaaDelta * 0.5;

  // Position = pixel's top-left corner
  double2 startPos = position + msaaOffset;
  float4 color = float4(0, 0, 0, 0);

  // Mix alpha and color differently. Avoids color fading on the edges
  int samples = resolution * resolution;
  int opaqueSamples = 0;

  for (int row = 0; row < resolution; ++row) {
    for (int col = 0; col < resolution; ++col) {
      double2 currentPos = startPos + msaaDelta * double2(col, row);

      float4 currentColor = CalculateColor(currentPos);
      color += currentColor;

      if (currentColor.a > 0.01) {
        ++opaqueSamples;
      }
    }
  }

  opaqueSamples = max(opaqueSamples, 1);
  return float4(color.rgb / opaqueSamples, color.a / samples);
}

PSOutput PSMain(VSOutput input) {
  PSOutput output;
  output.color = CalculateColorMSAA(input.position.xy, g_msaaLevel);
  //output.color = float4(1.0, 1.0, 1.0, 1.0);
  return output;
}
