#pragma once
#include "HLSLCommon.hlsli"
/*
 PathTracer Ray�� TraceRayȣ���� Wrapper
*/
float4 TracePathTracerRay(in float3 rayOrigin,in float3 rayDirection, in uint currentRayRecursionDepth)
{
    if (currentRayRecursionDepth >= MAX_RECURSION_DEPTH)
    {//max_depth�϶��� ambient light�� incoming light������ ���
        return float4(0.3f, 0.3f, 0.3f, 0.03f);
    }
    RayDesc ray;
    ray.Origin = rayOrigin;
    ray.Direction = rayDirection;
    ray.TMin = 0.001f;
    ray.TMax = 10000.0;
        
    
    PathTracerRayPayload payload = {
        float4(0, 0, 0, 0), 
        rayOrigin,
        currentRayRecursionDepth+1 
    };
    TraceRay(
        g_scene, //acceleration structure
        RAY_FLAG_NONE, //���� �÷��׸� ���� ����
        TraceRayParameters::InstanceMask, //instance mask
        TraceRayParameters::HitGroupOffset[RayType::PathTracer], //hit group base index����(����:trace ray���� index+ instance index + (geometry index * geometry stride)
        TraceRayParameters::GeometryStride, //geometry stride
        TraceRayParameters::MissShaderOffset[RayType::PathTracer], //miss shader index
        ray, //ray ����
        payload //payload
    );
    return payload.color;//closesthitshader, missshader�� ����� ������ incoming light����
}