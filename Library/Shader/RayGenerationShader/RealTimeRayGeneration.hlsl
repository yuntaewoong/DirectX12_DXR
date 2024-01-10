#define HLSL
#include "../Include/HLSLCommon.hlsli"
#include "../Include/RealTimeRayTrace.hlsli"
#include "../Include/RaygenerationShaderCommon.hlsli"


[shader("raygeneration")]
void RealTimeRaygenShader()
{
    float3 rayDir;
    float3 origin;
    GenerateWorldSpaceRay(DispatchRaysIndex().xy, origin, rayDir); //World Space Ray»ý¼º
    
    float4 color = float4(0.f, 0.f, 0.f, 1.f);
    uint currentRecursionDepth = 0u;
    color = TraceRealTimeRay(origin, rayDir, currentRecursionDepth);
    g_renderTarget[DispatchRaysIndex().xy] = color;
    
}