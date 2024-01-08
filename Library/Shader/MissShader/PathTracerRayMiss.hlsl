#define HLSL
#include "../Include/HLSLCommon.hlsli"

[shader("miss")]
void PathTracerRayMissShader(inout PathTracerRayPayload payload)
{
    payload.color = float4(0.03f,0.03f,0.03f,0.03f);
}