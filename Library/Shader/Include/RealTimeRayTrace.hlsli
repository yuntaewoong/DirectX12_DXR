#pragma once
#include "HLSLCommon.hlsli"

/*
 Radiance Ray의 Trace호출의 Wrapper(closest hit shader,ray generation shader에서 사용됨
*/

float4 TraceRealTimeRay(in float3 rayOrigin,in float3 rayDirection, in uint currentRayRecursionDepth)
{
    if (currentRayRecursionDepth >= MAX_RECURSION_DEPTH)
    {
        return float4(1.f, 1.f, 1.f, 0.0f);
    }
    RayDesc ray;
    ray.Origin = rayOrigin;
    ray.Direction = rayDirection;
    ray.TMin = 0.001f;
    ray.TMax = 10000.0;
        
    RealTimeRayPayload payload = { float4(0, 0, 0, 0), currentRayRecursionDepth+1 };
    TraceRay(
        g_scene, //acceleration structure
        RAY_FLAG_NONE, //딱히 플래그를 주지 않음
        TraceRayParameters::InstanceMask, //instance mask
        TraceRayParameters::HitGroupOffset[RayType::RealTime], //hit group base index설정(공식:trace ray설정 index+ instance index + (geometry index * geometry stride)
        TraceRayParameters::GeometryStride, //geometry stride
        TraceRayParameters::MissShaderOffset[RayType::RealTime], //miss shader index
        ray, //ray 정보
        payload //payload
    );
    return payload.color;
}