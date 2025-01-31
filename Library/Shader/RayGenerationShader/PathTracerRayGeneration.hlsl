#define HLSL
#include "../Include/HLSLCommon.hlsli"
#include "../Include/RaygenerationShaderCommon.hlsli"
#include "../Include/PathTracerRayTrace.hlsli"

[shader("raygeneration")]
void PathTracerRaygenShader()
{
    float3 rayDir;
    float3 origin;
    GenerateWorldSpaceRay(DispatchRaysIndex().xy, origin, rayDir); //World Space Ray����

	
    uint currentRecursionDepth = 0u;
    float3 Li = TracePathTracerRay(origin, rayDir, currentRecursionDepth).xyz;
	float3 prevLi = g_renderTarget[DispatchRaysIndex().xy].xyz;
	uint historyCount = g_sampleCountCB.sampleCount;
	Li = lerp(prevLi, Li, 1.0 / (1.0 + historyCount));
	g_renderTarget[DispatchRaysIndex().xy] = float4(Li,1);
}