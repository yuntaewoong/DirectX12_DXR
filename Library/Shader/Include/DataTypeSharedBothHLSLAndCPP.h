#pragma once

#ifndef DATA_TYPE_SHARED_BOTH_HLSL_AND_CPP_H
#define DATA_TYPE_SHARED_BOTH_HLSL_AND_CPP_H
#define NUM_LIGHT 2 //빛 개수
#define MAX_RECURSION_DEPTH 2 //TraceRay재귀 호출 depth

#ifdef HLSL//HLSL에서 include시
//전처리기를 이용해 HLSL에서도 c++에서도 같은 파일을 Include시켜주는 트릭
typedef float FLOAT;
typedef float2 XMFLOAT2;
typedef float3 XMFLOAT3;
typedef float4 XMFLOAT4;
typedef float4 XMVECTOR;
typedef float4x4 XMMATRIX;
typedef uint UINT;

#else//CPP에서 include시
#include <DirectXMath.h>
using namespace DirectX;
struct Index//Index구조체는 HLSL에서 필요없음(HLSL Index버퍼 형식인 ByteAddressBuffer는 기본 2바이트씩 Offset을 가짐)
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
    FLOAT hit; // 1이면 Hit, 0이면 Miss
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
        3  // Shadow ray Hit Group시작 인덱스, Renderable개수에 따라 다름
    };
    static const UINT GeometryStride = RayType::Count;
    static const UINT MissShaderOffset[RayType::Count] = {
        0, // Radiance ray
        1  // Shadow ray
    };
}



#endif