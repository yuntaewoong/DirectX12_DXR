#define HLSL
#include "../Include/HLSLCommon.hlsli"

[shader("miss")]
void PathTracerRayMissShader(inout PathTracerRayPayload payload)
{
    payload.color = float4(0.1f, 0.1f, 0.1f, 0.0f);
}