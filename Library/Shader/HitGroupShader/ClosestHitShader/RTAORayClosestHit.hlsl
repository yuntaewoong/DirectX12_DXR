#define HLSL
#include "../../Include/HLSLCommon.hlsli"
[shader("closesthit")]
void RTAORayClosestHitShader(inout RTAORayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    payload.tHit = RayTCurrent();
}