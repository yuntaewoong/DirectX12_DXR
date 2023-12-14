#define HLSL
#include "../../Include/HLSLCommon.hlsli"
#include "../../Include/RadianceRayTrace.hlsli"
    
[shader("closesthit")]
void PathTracerRayClosestHitShader(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    payload.color = float4(0,0,0, 1);
}