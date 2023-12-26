#pragma once
#include "HLSLCommon.hlsli"

/*
Shadow Ray�� Traceȣ���� Wrapper
*/

// Shadow Ray�� �̿��� �׸����̸� true, �ƴϸ� false����
float TraceShadowRay(in float3 hitPosition, in float4 lightPosition, in uint currentRayRecursionDepth)
{
    if (currentRayRecursionDepth >= MAX_RECURSION_DEPTH)
    {
        return false;
    }
    RayDesc rayDesc;
    ShadowRayPayload shadowPayload;
    rayDesc.Direction = normalize(lightPosition.xyz - hitPosition); //shadow ray ������ hit����->light����
    rayDesc.Origin = hitPosition;
    rayDesc.TMin = 0.001f;
    rayDesc.TMax = 10000.f;
    
    shadowPayload.hit = 1.f; //closest hit�� ������ �ʱ�ȭ
    TraceRay(
        g_scene, //acceleration structure
        RAY_FLAG_SKIP_CLOSEST_HIT_SHADER, //closest hit shader����(�ӵ� ���)(���߿� �ʿ��������� ���� ���ϰԵɵ�)
        TraceRayParameters::InstanceMask, //instance mask���� ���� x
        TraceRayParameters::HitGroupOffset[RayType::Shadow], //hit group index
        TraceRayParameters::GeometryStride, //shader table���� geometry�� ���� ����
        TraceRayParameters::MissShaderOffset[RayType::Shadow], //miss shader table���� ����� miss shader�� index
        rayDesc, //ray ����
        shadowPayload //payload
    );
    
    
    return shadowPayload.hit; 
}