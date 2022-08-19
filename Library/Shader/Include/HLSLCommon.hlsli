#pragma once
#include "DataTypeSharedBothHLSLAndCPP.h"

/*=================================================================
    g_(variable_name) ===> global root signature로 정의되는 Resource(매 프레임 1번씩 세팅)
==================================================================*/
//SRV
RaytracingAccelerationStructure g_scene : register(t0, space0);
//UAV
RWTexture2D<float4> g_renderTarget : register(u0);
//CBV
ConstantBuffer<CameraConstantBuffer> g_cameraCB : register(b0);
ConstantBuffer<PointLightConstantBuffer> g_lightCB : register(b2);//<<이 intellisense error는 무시해도 됨

