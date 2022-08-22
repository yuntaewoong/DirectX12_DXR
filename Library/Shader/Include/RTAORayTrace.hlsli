#pragma once
#include "HLSLCommon.hlsli"

/*
RTAO Trace Ray호출의 Wrapper
*/

float3 TraceRTAORay(in float3 hitPosition, in float3 hitNormal, in UINT currentRayRecursionDepth)
{
    /*RTAO Ray샘플링 전략: 본래 Random Direction을 설정해서(1개정도) 그 후 이전 프레임 데이터로 Denoising을 하는것이 가장 성능 부하를 덜 먹고 좋은 퀄리티를 뽑을 수 있으나
        내 프로젝트에서는 특정 Direction을 잡아서 Denoising없이 가는것으로 결정(간단하니까)
        특정 Direction에다가 Ray를 쏴주기 위해서는 hit position에 대응하는 TBN Matrix(World->tangent space)를 구해서(구하는법:Normal+ Normal에 직교하는 임의의 벡터+ 앞에 2놈 외적한 벡터를 축으로 삼음)
        Tangent Space에 정의된 Ray방향 * TBN Matrix해서 ray direction을 결정하면 됨
    */
    if (currentRayRecursionDepth >= MAX_RECURSION_DEPTH)
    {
        return float3(1.f, 1.f, 1.f);
    }
    float3x3 tbnMatrix = 0;
    {
        float3 seedVector = normalize(float3(1.f, 2.f, 3.f)); //hitNormal과 수직인 임의의 벡터를 구하기 위한 Seed값(아무값이나 써도댐)
        float3 tangent = normalize(seedVector - hitNormal * dot(seedVector, hitNormal));
        float3 biTangent = normalize(cross(tangent, hitNormal));
        tbnMatrix = float3x3(tangent, biTangent, normalize(hitNormal)); //tangent space벡터=>world space벡터 변환행렬 완성
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
            RAY_FLAG_NONE, //딱히 플래그를 주지 않음
            TraceRayParameters::InstanceMask, //instance mask
            TraceRayParameters::HitGroupOffset[RayType::RTAO], //hit group base index설정(공식:trace ray설정 index+ instance index + (geometry index * geometry stride)
            TraceRayParameters::GeometryStride, //geometry stride
            TraceRayParameters::MissShaderOffset[RayType::RTAO], //miss shader index
            rayDesc, //ray 정보
            payload //payload
        );
        ambientOcclusion += payload.tHit / float(RTAO_RADIUS); //가까운 곳에 물체가 있어서 Radius에 대한 tHit비율이 보다 작아질수록 AmbientOcclusion값은 어두워짐(0에가까워짐)
    }
    ambientOcclusion /= float(NUM_RTAO_RAY);
    return saturate(float3(ambientOcclusion, ambientOcclusion, ambientOcclusion));
}