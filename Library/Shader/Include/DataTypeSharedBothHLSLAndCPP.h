#pragma once

#ifndef DATA_TYPE_SHARED_BOTH_HLSL_AND_CPP_H
#define DATA_TYPE_SHARED_BOTH_HLSL_AND_CPP_H
#define NUM_LIGHT 2 //�� ����
#define MAX_RECURSION_DEPTH 15 //TraceRay��� ȣ�� depth

#ifdef HLSL//HLSL���� include��
//��ó���⸦ �̿��� HLSL������ c++������ ���� ������ Include�����ִ� Ʈ��
typedef float FLOAT;
typedef float2 XMFLOAT2;
typedef float3 XMFLOAT3;
typedef float4 XMFLOAT4;
typedef float4 XMVECTOR;
typedef float4x4 XMMATRIX;
typedef uint UINT;

#else//CPP���� include��
#include <DirectXMath.h>
using namespace DirectX;
struct Index//Index����ü�� HLSL���� �ʿ����(HLSL Index���� ������ ByteAddressBuffer�� �⺻ 2����Ʈ�� Offset�� ����)
{
    UINT16 index;
};

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
struct RenderableConstantBuffer
{
    XMMATRIX world;
    XMFLOAT4 albedo;
    UINT hasTexture;//1�̸� texture����, 0�̸� ����
    FLOAT reflectivity;//1�̸� �ſ�,0�̸� ���ݻ�
    FLOAT padding[2];//32��Ʈ 4������ ���߱� ���� padding
};
struct Vertex
{
    XMFLOAT3 position;
    XMFLOAT2 uv;
    XMFLOAT3 normal;
};
struct RayPayload
{
    XMFLOAT4 color;
    UINT recursionDepth;//�󸶳� ���ȣ��Ǿ��°�?
};

struct ShadowRayPayload
{
    FLOAT hit; // 1�̸� Hit, 0�̸� Miss
};
namespace RayType 
{
    enum Enum {
        Radiance = 0,   
        Shadow         
    };
    static const UINT Count = 2;
}

namespace TraceRayParameters
{
    static const UINT InstanceMask = ~0; 
    static const UINT HitGroupOffset[RayType::Count] = {
        0, // Radiance ray
        1  // Shadow ray
    };
    static const UINT GeometryStride = RayType::Count;
    static const UINT MissShaderOffset[RayType::Count] = {
        0, // Radiance ray
        1  // Shadow ray
    };
}



#endif