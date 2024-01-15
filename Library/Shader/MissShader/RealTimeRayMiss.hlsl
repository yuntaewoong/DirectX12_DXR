#define HLSL
#include "../Include/HLSLCommon.hlsli"

[shader("miss")]
void RealTimeRayMissShader(inout RealTimeRayPayload payload)
{
    payload.color = float4(0.03f, 0.03f, 0.03f, 1.0f);
}