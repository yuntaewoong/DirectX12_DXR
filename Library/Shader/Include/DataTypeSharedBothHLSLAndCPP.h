#ifndef DATA_TYPE_SHARED_BOTH_HLSL_AND_CPP_H
#define DATA_TYPE_SHARED_BOTH_HLSL_AND_CPP_H

#define NUM_LIGHT 2 //�� ����
#define MAX_RECURSION_DEPTH 20 //TraceRay��� ȣ�� depth
#define NUM_RTAO_RAY 4 // RTAO�� ����� Ray��
#define RTAO_RADIUS 0.2  // RTAO�� ������ �ݱ��� ������


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

struct MeshConstantBuffer
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
    XMFLOAT3 tangent;
    XMFLOAT3 biTangent;
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

struct RTAORayPayload
{
    FLOAT tHit; // at + b���� �������� t��
};

namespace RayType 
{
    enum Enum {
        Radiance = 0,   
        Shadow,
        RTAO
    };
    static const UINT Count = 3;
}

namespace TraceRayParameters
{
    static const UINT InstanceMask = ~0; 
    static const UINT HitGroupOffset[RayType::Count] = {
        0, // Radiance ray
        1, // Shadow ray
        2  // RTAO ray
    };
    static const UINT GeometryStride = RayType::Count;
    static const UINT MissShaderOffset[RayType::Count] = {
        0, // Radiance ray
        1, // Shadow ray
        2  // RTAO ray
    };
}



#endif