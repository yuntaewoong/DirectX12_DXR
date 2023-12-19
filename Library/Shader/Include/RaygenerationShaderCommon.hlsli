#pragma once
#include "HLSLCommon.hlsli"
// Screen Space Ray�� World Space Ray�� ��ȯ
// index: screen ��ǥ(�ػ� ���� ����), origin: world��ǥ,  direction: world����
inline void GenerateWorldSpaceRay(in uint2 rayIndex, out float3 origin, out float3 direction)
{
    float2 xy = rayIndex + 0.5f; // �ȼ��� ���� ä��
    float2 screenPos = xy / DispatchRaysDimensions().xy * 2.0 - 1.0; //NDC��ǥ��� ��ȯ(�� ����: -1 ~ 1)

    screenPos.y = -screenPos.y; //directX �� LHS��ǥ��� �ȼ� index���� y���� ������ �ٸ��� ������ ����
    
    float4 world = mul(float4(screenPos, 0, 1), g_cameraCB.projectionToWorld); //NDC��ǥ�� Ray�� �������� World Space�� ��ȯ

    world.xyz /= world.w;
    origin = g_cameraCB.cameraPosition.xyz; //origin�� ī�޶� ��ǥ
    direction = normalize(world.xyz - origin); //direction�� world-camera
}
