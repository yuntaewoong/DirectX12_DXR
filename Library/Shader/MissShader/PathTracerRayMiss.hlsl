#define HLSL
#include "../Include/HLSLCommon.hlsli"

[shader("miss")]
void PathTracerRayMissShader(inout PathTracerRayPayload payload)
{
    payload.color = float4(0.3f,0.3f,0.3f,1.f);
}