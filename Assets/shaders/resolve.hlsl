Texture2DMS<float4> targetTexture : register(t0);

struct VSOutput {
  float4 position : SV_POSITION;
  float2 uv : UV;
};

VSOutput VSMain(uint vertexId : SV_VERTEXID) {
  VSOutput result;

  // TopLeft
  if (vertexId == 0) {
    result.position = float4(-1.0, 3.0, 0.0, 1.0);
    result.uv = float2(0, -1);
    return result;
  }

  // BottomRight
  if (vertexId == 1) {
    result.position = float4(3.0, -1.0, 0.0, 1.0);
    result.uv = float2(2, 1);
    return result;
  }

  // BottomLeft
  if (vertexId == 2) {
    result.position = float4(-1.0, -1.0, 0.0, 1.0);
    result.uv = float2(0, 1);
    return result;
  }

  return result;
}

float4 PSMain(VSOutput input) : SV_TARGET {
  float4 color = 0.0.xxxx;
  uint width;
  uint height;
  uint samples;
  targetTexture.GetDimensions(width, height, samples);

  for (uint sample = 0; sample < samples; ++sample) {
	  color += targetTexture.Load(input.uv * float2(width, height), sample);
  }

  color /= samples;
  return color;
}
