#pragma once
#include "DataTypeSharedBothHLSLAndCPP.h"

namespace RTAORayDirection
{//RTAO Trace Ray���� ����� ���� ���͵�
    static const float3 directions[NUM_RTAO_RAY] =
    {
        float3(1.f, 1.f, 1.f),
        float3(1.f, -1.f, 1.f),
        float3(-1.f, 1.f, 1.f),
        float3(-1.f, -1.f, 1.f)
    };
}

/*=================================================================
    g_(variable_name) ===> global root signature�� ���ǵǴ� Resource(�� ������ 1���� ����)
==================================================================*/
//SRV
RaytracingAccelerationStructure g_scene : register(t0, space0);
//UAV
RWTexture2D<float4> g_renderTarget : register(u0);
//CBV

ConstantBuffer<CameraConstantBuffer> g_cameraCB : register(b0);
ConstantBuffer<RandomConstantBuffer> g_randomCB : register(b2);
ConstantBuffer<PointLightConstantBuffer> g_pointLightCB : register(b3);
ConstantBuffer<AreaLightConstantBuffer> g_areaLightCB : register(b4);
ConstantBuffer<RandomSampleCounterConstantBuffer>  g_sampleCountCB : register(b5);
//<<�� intellisense error�� �����ص� ��

