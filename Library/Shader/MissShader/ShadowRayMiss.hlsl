
#define HLSL
#include "../Include/HLSLCommon.hlsli"

[shader("miss")]
void ShadowRayMissShader(inout ShadowRayPayload payload)
{
    payload.hit = 0.f; //안맞았다!(그림자o)
}