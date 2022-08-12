#pragma once

#ifndef DATA_TYPE_SHARED_BOTH_HLSL_AND_CPP_H
#define DATA_TYPE_SHARED_BOTH_HLSL_AND_CPP_H
#define NUM_LIGHT 2 //�� ����
#define MAX_RECURSION_DEPTH 2 //TraceRay��� ȣ�� depth

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
struct CubeConstantBuffer
{
    XMFLOAT4 albedo;
};
struct Vertex
{
    XMFLOAT3 position;
    XMFLOAT3 normal;
};
struct RayPayload
{
    XMFLOAT4 color;
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
        3  // Shadow ray Hit Group���� �ε���, Renderable������ ���� �ٸ�
    };
    static const UINT GeometryStride = RayType::Count;
    static const UINT MissShaderOffset[RayType::Count] = {
        0, // Radiance ray
        1  // Shadow ray
    };
}



#endif