#define HLSL
#include "../Include/HLSLCommon.hlsli"
#include "../Include/RadianceRayTrace.hlsli"

// Screen Space Ray를 World Space Ray로 변환
// index: screen 좌표(해상도 별로 상이), origin: world좌표,  direction: world방향
inline void GenerateCameraRay(uint2 index, out float3 origin, out float3 direction)
{
    float2 xy = index + 0.5f; // 픽셀의 중점 채용
    float2 screenPos = xy / DispatchRaysDimensions().xy * 2.0 - 1.0; //NDC좌표계로 변환(값 범위: -1 ~ 1)

    screenPos.y = -screenPos.y; //directX 의 LHS좌표계는 픽셀 index값과 y값의 방향이 다르기 때문에 조정
    
    float4 world = mul(float4(screenPos, 0, 1), g_cameraCB.projectionToWorld); //NDC좌표계 Ray의 목적지를 World Space로 변환

    world.xyz /= world.w;
    origin = g_cameraCB.cameraPosition.xyz; //origin의 카메라 좌표
    direction = normalize(world.xyz - origin); //direction은 world-camera
}


[shader("raygeneration")]
void RadianceRaygenShader()
{
    float3 rayDir;
    float3 origin;
    GenerateCameraRay(DispatchRaysIndex().xy, origin, rayDir); //World Space Ray생성
    
    float4 color = float4(0.f, 0.f, 0.f, 1.f);
    uint currentRecursionDepth = 0u;
    color = TraceRadianceRay(origin, rayDir, currentRecursionDepth);
    g_renderTarget[DispatchRaysIndex().xy] = color;

}