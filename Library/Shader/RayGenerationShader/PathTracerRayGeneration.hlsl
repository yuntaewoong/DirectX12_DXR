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

	
    uint currentRecursionDepth = 0u;
    float3 Li = TracePathTracerRay(origin, rayDir, currentRecursionDepth).xyz;
	float3 prevLi = g_renderTarget[DispatchRaysIndex().xy].xyz;
	float historyCount = g_renderTarget[DispatchRaysIndex().xy].w;
	Li = lerp(prevLi, Li, 1.0 / (1.0 + historyCount));
	historyCount += 1;

	g_renderTarget[DispatchRaysIndex().xy] = float4(Li,historyCount);
}