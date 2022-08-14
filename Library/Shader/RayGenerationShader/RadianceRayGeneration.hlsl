#define HLSL
#include "../Include/HLSLCommon.hlsli"


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
void MyRaygenShader()
{
    float3 rayDir;
    float3 origin;
    GenerateCameraRay(DispatchRaysIndex().xy, origin, rayDir); //World Space Ray생성
    
    
    RayDesc ray;
    ray.Origin = origin;
    ray.Direction = rayDir;
    ray.TMin = 0.001f;
    ray.TMax = 10000.0;
    RayPayload payload = { float4(0, 0, 0, 0) };
    TraceRay(
        g_scene, //acceleration structure
        RAY_FLAG_NONE, //딱히 플래그를 주지 않음
        TraceRayParameters::InstanceMask, //instance mask
        TraceRayParameters::HitGroupOffset[RayType::Radiance], //hit group base index설정(공식:trace ray설정 index+ instance index + (geometry index * geometry stride)
        TraceRayParameters::GeometryStride, //geometry stride
        TraceRayParameters::MissShaderOffset[RayType::Radiance], //miss shader index
        ray, //ray 정보
        payload //payload
    );
 
    g_renderTarget[DispatchRaysIndex().xy] = payload.color;

}