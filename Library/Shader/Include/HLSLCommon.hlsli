#include "DataTypeSharedBothHLSLAndCPP.h"
/*=================================================================
    g_(variable_name) ===> global root signature�� ���ǵǴ� Resource(�� ������ 1���� ����)
==================================================================*/
RaytracingAccelerationStructure g_scene : register(t0, space0);
RWTexture2D<float4> g_renderTarget : register(u0);
ConstantBuffer<CameraConstantBuffer> g_cameraCB : register(b0);
ConstantBuffer<PointLightConstantBuffer> g_lightCB : register(b2);//<<�� intellisense error�� �����ص� ��

