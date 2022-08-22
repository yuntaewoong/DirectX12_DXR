#pragma once
#include "HLSLCommon.hlsli"

/*
RTAO Trace Rayȣ���� Wrapper
*/

float3 TraceRTAORay(in float3 hitPosition, in float3 hitNormal, in UINT currentRayRecursionDepth)
{
    /*RTAO Ray���ø� ����: ���� Random Direction�� �����ؼ�(1������) �� �� ���� ������ �����ͷ� Denoising�� �ϴ°��� ���� ���� ���ϸ� �� �԰� ���� ����Ƽ�� ���� �� ������
        �� ������Ʈ������ Ư�� Direction�� ��Ƽ� Denoising���� ���°����� ����(�����ϴϱ�)
        Ư�� Direction���ٰ� Ray�� ���ֱ� ���ؼ��� hit position�� �����ϴ� TBN Matrix(World->tangent space)�� ���ؼ�(���ϴ¹�:Normal+ Normal�� �����ϴ� ������ ����+ �տ� 2�� ������ ���͸� ������ ����)
        Tangent Space�� ���ǵ� Ray���� * TBN Matrix�ؼ� ray direction�� �����ϸ� ��
    */
    if (currentRayRecursionDepth >= MAX_RECURSION_DEPTH)
    {
        return float3(1.f, 1.f, 1.f);
    }
    float3x3 tbnMatrix = 0;
    {
        float3 seedVector = normalize(float3(1.f, 2.f, 3.f)); //hitNormal�� ������ ������ ���͸� ���ϱ� ���� Seed��(�ƹ����̳� �ᵵ��)
        float3 tangent = normalize(seedVector - hitNormal * dot(seedVector, hitNormal));
        float3 biTangent = normalize(cross(tangent, hitNormal));
        tbnMatrix = float3x3(tangent, biTangent, normalize(hitNormal)); //tangent space����=>world space���� ��ȯ��� �ϼ�
    }
    float ambientOcclusion = 0.f;
        
    [unroll(NUM_RTAO_RAY)]
    for (uint i = 0; i < NUM_RTAO_RAY;i++)
    {
        
        RayDesc rayDesc;
        rayDesc.Direction = normalize(mul(RTAORayDirection::directions[i].xyz, tbnMatrix));
        rayDesc.Origin = hitPosition;
        rayDesc.TMin = 0.001f;
        rayDesc.TMax = RTAO_RADIUS;
            
        RTAORayPayload payload = { float(RTAO_RADIUS) };
        TraceRay(
            g_scene, //acceleration structure
            RAY_FLAG_NONE, //���� �÷��׸� ���� ����
            TraceRayParameters::InstanceMask, //instance mask
            TraceRayParameters::HitGroupOffset[RayType::RTAO], //hit group base index����(����:trace ray���� index+ instance index + (geometry index * geometry stride)
            TraceRayParameters::GeometryStride, //geometry stride
            TraceRayParameters::MissShaderOffset[RayType::RTAO], //miss shader index
            rayDesc, //ray ����
            payload //payload
        );
        ambientOcclusion += payload.tHit / float(RTAO_RADIUS); //����� ���� ��ü�� �־ Radius�� ���� tHit������ ���� �۾������� AmbientOcclusion���� ��ο���(0���������)
    }
    ambientOcclusion /= float(NUM_RTAO_RAY);
    return saturate(float3(ambientOcclusion, ambientOcclusion, ambientOcclusion));
}