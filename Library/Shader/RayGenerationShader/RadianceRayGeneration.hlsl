#define HLSL
#include "../Include/HLSLCommon.hlsli"
#include "../Include/RadianceRayTrace.hlsli"

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
void RadianceRaygenShader()
{
    float3 rayDir;
    float3 origin;
    GenerateCameraRay(DispatchRaysIndex().xy, origin, rayDir); //World Space Ray����
    
    float4 color = float4(0.f, 0.f, 0.f, 1.f);
    uint currentRecursionDepth = 0u;
    color = TraceRadianceRay(origin, rayDir, currentRecursionDepth);
    g_renderTarget[DispatchRaysIndex().xy] = color;

}