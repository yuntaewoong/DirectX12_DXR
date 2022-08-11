#pragma once

#ifndef DATA_TYPE_SHARED_BOTH_HLSL_AND_CPP_H
#define DATA_TYPE_SHARED_BOTH_HLSL_AND_CPP_H

#define NUM_LIGHT 2 //�� ����
#define MAX_RECURSION_DEPTH 2 //TraceRay��� ȣ�� depth


#ifdef HLSL//HLSL������ Include�����ִ� trick
typedef float FLOAT;
typedef float2 XMFLOAT2;
typedef float3 XMFLOAT3;
typedef float4 XMFLOAT4;
typedef float4 XMVECTOR;
typedef float4x4 XMMATRIX;
typedef uint UINT16;


#else//CPP���� include��
#include <DirectXMath.h>
using namespace DirectX;

#endif

struct CameraConstantBuffer
{
    XMMATRIX projectionToWorld;
    XMVECTOR cameraPosition;
};
struct PointLightConstantBuffer
{
    XMVECTOR position[NUM_LIGHT];
};
struct CubeConstantBuffer
{
    XMFLOAT4 albedo;
};
struct Vertex
{
    XMFLOAT3 position;
    XMFLOAT3 normal;
};
struct Index
{
    UINT16 index;
};
struct RayPayload
{
    XMFLOAT4 color;
};

struct ShadowRayPayload
{
    FLOAT hit; // 1�̸� Hit, 0�̸� Miss
};



#endif