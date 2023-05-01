#pragma once
#include "HLSLCommon.hlsli"

/*
Shadow Ray의 Trace호출의 Wrapper
*/

// Shadow Ray를 이용해 그림자이면 true, 아니면 false리턴
float TraceShadowRay(in float3 hitPosition, in float4 lightPositions[NUM_LIGHT], in UINT currentRayRecursionDepth)
{
    if (currentRayRecursionDepth >= MAX_RECURSION_DEPTH)
    {
        return false;
    }
    RayDesc rayDescs[NUM_LIGHT];
    ShadowRayPayload shadowPayloads[NUM_LIGHT];
    [unroll(NUM_LIGHT)]
    for (uint i = 0; i < NUM_LIGHT; i++)
    {
        rayDescs[i].Direction = normalize(lightPositions[i].xyz - hitPosition); //shadow ray 방향은 hit지점->light지점
        rayDescs[i].Origin = hitPosition;
        rayDescs[i].TMin = 0.001f;
        rayDescs[i].TMax = 10000.f;
    
    
        shadowPayloads[i].hit = 1.f; //closest hit시 값으로 초기화
    
        TraceRay(
            g_scene, //acceleration structure
            RAY_FLAG_SKIP_CLOSEST_HIT_SHADER, //closest hit shader무시(속도 향상)(나중에 필요해질때는 무시 안하게될듯)
            TraceRayParameters::InstanceMask, //instance mask딱히 설정 x
            TraceRayParameters::HitGroupOffset[RayType::Shadow], //hit group index
            TraceRayParameters::GeometryStride, //shader table에서 geometry들 간의 간격
            TraceRayParameters::MissShaderOffset[RayType::Shadow], //miss shader table에서 사용할 miss shader의 index
            rayDescs[i], //ray 정보
            shadowPayloads[i] //payload
        );
    }
    
    return shadowPayloads[0].hit + shadowPayloads[1].hit; 
}