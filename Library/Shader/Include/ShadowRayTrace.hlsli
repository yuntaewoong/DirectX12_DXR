#pragma once
#include "HLSLCommon.hlsli"

/*
Shadow Ray�� Traceȣ���� Wrapper
*/

// Shadow Ray�� �̿��� �׸����̸� true, �ƴϸ� false����
bool TraceShadowRay(in float3 hitPosition, in float3 lightPosition, in UINT currentRayRecursionDepth)
{
    if (currentRayRecursionDepth >= MAX_RECURSION_DEPTH)
    {
        return false;
    }
    RayDesc rayDesc;
    rayDesc.Direction = normalize(lightPosition - hitPosition); //shadow ray ������ hit����->light����
    rayDesc.Origin = hitPosition;
    rayDesc.TMin = 0.001f;
    rayDesc.TMax = 10000.f;
    
    ShadowRayPayload shadowPayload;
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
    return shadowPayload.hit > 0.5f; //0.5f���� ũ�ٴ� �ǹ̴� miss shader�� ȣ����� �ʾƼ� �׸��� ������ �����Ѵٴ� �ǹ�
}