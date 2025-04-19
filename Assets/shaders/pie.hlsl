#define PI 3.14159265358976f

cbuffer FrameData : register(b0)
{
  float2 g_resolution;
};

// Vertex

struct VSInput {
  uint vertexId : SV_VertexID;
};

struct VSOutput {
  float4 position : SV_POSITION;
  float4 positionNomalized : POSITION_NORMALIZED;
};

VSOutput VSMain(VSInput input) {
  VSOutput output;

  float radius = min(g_resolution.x, g_resolution.y) * 0.324;
  float2 center = g_resolution / 2;

  // Bottom-Left
  if (input.vertexId == 2) {
    output.position = float4(
      ((center.x - radius) / g_resolution.x) * 2 - 1,
      ((center.y - radius) / g_resolution.y) * 2 - 1,
      0.0f,
      1.0f
    );
    //output.position = float4(-1.0f, -1.0f, 0.0f, 1.0f);
  }

  // Bottom-Right
  if (input.vertexId == 1) {
    output.position = float4(
      ((center.x + radius * 3) / g_resolution.x) * 2 - 1,
      ((center.y - radius) / g_resolution.y) * 2 - 1,
      0.0f,
      1.0f
    );
    //output.position = float4(3.0f, -1.0f, 0.0f, 1.0f);
  }

  // Top-Left
  if (input.vertexId == 0) {
    output.position = float4(
      ((center.x - radius) / g_resolution.x) * 2 - 1,
      ((center.y + radius * 3) / g_resolution.y) * 2 - 1,
      0.0f,
      1.0f
    );
    //output.position = float4(-1.0f, 3.0f, 0.0f, 1.0f);
  }

  output.positionNomalized = output.position;

  return output;
}

// Pixel

struct PSOutput {
  float4 color : SV_TARGET;
};

PSOutput PSMain(VSOutput input) {
  PSOutput output;
  float2 position = input.position.xy;
  float2 uv = position / g_resolution;
  float2 uvCS = (position / g_resolution) * 2 - 1;

  float radius = min(g_resolution.x, g_resolution.y) * 0.324;
  float smallRadius = min(50, 0.15 * radius);
  float2 center = g_resolution / 2;
  float2 centerVec = position - center;

  // Out of bounds
  if (sqrt(centerVec.x * centerVec.x + centerVec.y * centerVec.y) > radius) {
    discard;
  }

  // Small circle
  if (sqrt(centerVec.x * centerVec.x + centerVec.y * centerVec.y) < smallRadius) {
    output.color = float4(0.11328125, 0.11328125, 0.11328125, 1.0);
    return output;
  }

  // Segment
  int segments = 4;
  int segment = 0;
  float segmentSize = 2 * PI / segments;

  float pixelAngle = atan2(centerVec.y, centerVec.x);
  if (pixelAngle < 0.0f) {
    pixelAngle += 2.0f * PI;
  }

  int pixelSegment = ((int)(pixelAngle / segmentSize) + 1) % segments;

  if (pixelSegment == segment) {
    output.color = float4(0.11328125, 0.11328125, 0.11328125, 1.0);
    return output;
  }

  // Big circle
  output.color = float4(0.03125, 0.03125, 0.03125, 1.0);
  return output;
}