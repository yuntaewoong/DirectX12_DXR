#define HLSL
#include "../Include/HLSLCommon.hlsli"


// Screen Space Ray�� World Space Ray�� ��ȯ
// index: screen ��ǥ(�ػ� ���� ����), origin: world��ǥ,  direction: world����
inline void GenerateCameraRay(uint2 index, out float3 origin, out float3 direction)
{
    float2 xy = index + 0.5f; // �ȼ��� ���� ä��
    float2 screenPos = xy / DispatchRaysDimensions().xy * 2.0 - 1.0; //NDC��ǥ��� ��ȯ(�� ����: -1 ~ 1)

    screenPos.y = -screenPos.y; //directX �� LHS��ǥ��� �ȼ� index���� y���� ������ �ٸ��� ������ ����
    
    float4 world = mul(float4(screenPos, 0, 1), g_cameraCB.projectionToWorld); //NDC��ǥ�� Ray�� �������� World Space�� ��ȯ

    world.xyz /= world.w;
    origin = g_cameraCB.cameraPosition.xyz; //origin�� ī�޶� ��ǥ
    direction = normalize(world.xyz - origin); //direction�� world-camera
}


[shader("raygeneration")]
void MyRaygenShader()
{
    float3 rayDir;
    float3 origin;
    GenerateCameraRay(DispatchRaysIndex().xy, origin, rayDir); //World Space Ray����
    
    
    RayDesc ray;
    ray.Origin = origin;
    ray.Direction = rayDir;
    ray.TMin = 0.001f;
    ray.TMax = 10000.0;
    RayPayload payload = { float4(0, 0, 0, 0) };
    TraceRay(
        g_scene, //acceleration structure
        RAY_FLAG_NONE, //���� �÷��׸� ���� ����
        TraceRayParameters::InstanceMask, //instance mask
        TraceRayParameters::HitGroupOffset[RayType::Radiance], //hit group base index����(����:trace ray���� index+ instance index + (geometry index * geometry stride)
        TraceRayParameters::GeometryStride, //geometry stride
        TraceRayParameters::MissShaderOffset[RayType::Radiance], //miss shader index
        ray, //ray ����
        payload //payload
    );
 
    g_renderTarget[DispatchRaysIndex().xy] = payload.color;

}