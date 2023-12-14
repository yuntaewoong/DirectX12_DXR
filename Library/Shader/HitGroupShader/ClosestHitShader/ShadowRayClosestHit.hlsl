#define HLSL
#include "../../Include/HLSLCommon.hlsli"

[shader("closesthit")]
void ShadowRayClosestHitShader(inout ShadowRayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    payload.hit = 1.f / NUM_LIGHT; //맞았다!(그림자x)
}