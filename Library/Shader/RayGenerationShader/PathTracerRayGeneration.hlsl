#define HLSL
#include "../Include/HLSLCommon.hlsli"
#include "../Include/RaygenerationShaderCommon.hlsli"
#include "../Include/PathTracerRayTrace.hlsli"

[shader("raygeneration")]
void PathTracerRaygenShader()
{
    float3 rayDir;
    float3 origin;
    GenerateWorldSpaceRay(DispatchRaysIndex().xy, origin, rayDir); //World Space Ray»ý¼º


    float4 color = float4(0.f, 0.f, 0.f, 1.f);
    uint currentRecursionDepth = 0u;
    color = TracePathTracerRay(origin, rayDir, currentRecursionDepth);
    g_renderTarget[DispatchRaysIndex().xy] = color;
        
        
}