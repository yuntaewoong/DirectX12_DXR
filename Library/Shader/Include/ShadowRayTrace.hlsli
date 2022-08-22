#pragma once
#include "HLSLCommon.hlsli"

/*
Shadow Ray의 Trace호출의 Wrapper
*/

// Shadow Ray를 이용해 그림자이면 true, 아니면 false리턴
bool TraceShadowRay(in float3 hitPosition, in float3 lightPosition, in UINT currentRayRecursionDepth)
{
    if (currentRayRecursionDepth >= MAX_RECURSION_DEPTH)
    {
        return false;
    }
    RayDesc rayDesc;
    rayDesc.Direction = normalize(lightPosition - hitPosition); //shadow ray 방향은 hit지점->light지점
    rayDesc.Origin = hitPosition;
    rayDesc.TMin = 0.001f;
    rayDesc.TMax = 10000.f;
    
    ShadowRayPayload shadowPayload;
    shadowPayload.hit = 1.f; //closest hit시 값으로 초기화
    
    TraceRay(
        g_scene, //acceleration structure
        RAY_FLAG_SKIP_CLOSEST_HIT_SHADER, //closest hit shader무시(속도 향상)(나중에 필요해질때는 무시 안하게될듯)
        TraceRayParameters::InstanceMask, //instance mask딱히 설정 x
        TraceRayParameters::HitGroupOffset[RayType::Shadow], //hit group index
        TraceRayParameters::GeometryStride, //shader table에서 geometry들 간의 간격
        TraceRayParameters::MissShaderOffset[RayType::Shadow], //miss shader table에서 사용할 miss shader의 index
        rayDesc, //ray 정보
        shadowPayload //payload
    );
    return shadowPayload.hit > 0.5f; //0.5f보다 크다는 의미는 miss shader가 호출되지 않아서 그림자 영역에 존재한다는 의미
}