#pragma once
#include "HLSLCommon.hlsli"
/*
 PathTracer Ray의 TraceRay호출의 Wrapper
*/
float4 TracePathTracerRay(in float3 rayOrigin,in float3 rayDirection, in uint currentRayRecursionDepth)
{
    if (currentRayRecursionDepth >= MAX_RECURSION_DEPTH)
    {
        return float4(0.f, 0.f, 0.f, 0.0f);
    }
    RayDesc ray;
    ray.Origin = rayOrigin;
    ray.Direction = rayDirection;
    ray.TMin = 0.001f;
    ray.TMax = 10000.0;
        
    RayPayload payload = { float4(0, 0, 0, 0), currentRayRecursionDepth+1 };
    TraceRay(
        g_scene, //acceleration structure
        RAY_FLAG_NONE, //딱히 플래그를 주지 않음
        TraceRayParameters::InstanceMask, //instance mask
        TraceRayParameters::HitGroupOffset[RayType::PathTracer], //hit group base index설정(공식:trace ray설정 index+ instance index + (geometry index * geometry stride)
        TraceRayParameters::GeometryStride, //geometry stride
        TraceRayParameters::MissShaderOffset[RayType::PathTracer], //miss shader index
        ray, //ray 정보
        payload //payload
    );
    return payload.color;
}