#define HLSL
#include "../Include/HLSLCommon.hlsli"

[shader("miss")]
void RadianceRayMissShader(inout RayPayload payload)
{
    payload.color = float4(0.0f, 0.2f, 0.4f, 1.0f);
}