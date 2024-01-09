#pragma once
#include "DataTypeSharedBothHLSLAndCPP.h"

namespace RTAORayDirection
{//RTAO Trace Ray에서 사용할 방향 벡터들
    static const float3 directions[NUM_RTAO_RAY] =
    {
        float3(1.f, 1.f, 1.f),
        float3(1.f, -1.f, 1.f),
        float3(-1.f, 1.f, 1.f),
        float3(-1.f, -1.f, 1.f)
    };
}

/*=================================================================
    g_(variable_name) ===> global root signature로 정의되는 Resource(매 프레임 1번씩 세팅)
==================================================================*/
//SRV
RaytracingAccelerationStructure g_scene : register(t0, space0);
//UAV
RWTexture2D<float4> g_renderTarget : register(u0);
//CBV
ConstantBuffer<CameraConstantBuffer> g_cameraCB : register(b0);
ConstantBuffer<PointLightConstantBuffer> g_lightCB : register(b2);
ConstantBuffer<RandomConstantBuffer> g_randomCB : register(b7);

//<<이 intellisense error는 무시해도 됨

