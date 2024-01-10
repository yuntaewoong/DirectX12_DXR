#pragma once
#include "HLSLCommon.hlsli"

/*
 Radiance Ray�� Traceȣ���� Wrapper(closest hit shader,ray generation shader���� ����
*/

float4 TraceRadianceRay(in float3 rayOrigin,in float3 rayDirection, in uint currentRayRecursionDepth)
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
        
    RayPayload payload = { float4(0, 0, 0, 0), currentRayRecursionDepth+1 };
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
    return payload.color;
}