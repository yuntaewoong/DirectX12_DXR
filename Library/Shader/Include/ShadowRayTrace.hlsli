#pragma once
#include "HLSLCommon.hlsli"

/*
Shadow Ray�� Traceȣ���� Wrapper
*/

// Shadow Ray�� �̿��� �׸����̸� true, �ƴϸ� false����
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
        rayDescs[i].Direction = normalize(lightPositions[i].xyz - hitPosition); //shadow ray ������ hit����->light����
        rayDescs[i].Origin = hitPosition;
        rayDescs[i].TMin = 0.001f;
        rayDescs[i].TMax = 10000.f;
    
    
        shadowPayloads[i].hit = 1.f; //closest hit�� ������ �ʱ�ȭ
    
        TraceRay(
            g_scene, //acceleration structure
            RAY_FLAG_SKIP_CLOSEST_HIT_SHADER, //closest hit shader����(�ӵ� ���)(���߿� �ʿ��������� ���� ���ϰԵɵ�)
            TraceRayParameters::InstanceMask, //instance mask���� ���� x
            TraceRayParameters::HitGroupOffset[RayType::Shadow], //hit group index
            TraceRayParameters::GeometryStride, //shader table���� geometry�� ���� ����
            TraceRayParameters::MissShaderOffset[RayType::Shadow], //miss shader table���� ����� miss shader�� index
            rayDescs[i], //ray ����
            shadowPayloads[i] //payload
        );
    }
    
    return shadowPayloads[0].hit + shadowPayloads[1].hit; 
}