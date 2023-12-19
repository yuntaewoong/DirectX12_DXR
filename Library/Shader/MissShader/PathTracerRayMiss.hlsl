#define HLSL
#include "../Include/HLSLCommon.hlsli"

[shader("miss")]
void PathTracerRayMissShader(inout RayPayload payload)
{
    payload.color = float4(1.f, 0.f, 0.0f, 1.0f);
}