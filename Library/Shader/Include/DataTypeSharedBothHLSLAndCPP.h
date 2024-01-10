#ifndef DATA_TYPE_SHARED_BOTH_HLSL_AND_CPP_H
#define DATA_TYPE_SHARED_BOTH_HLSL_AND_CPP_H

#define NUM_POINT_LIGHT_MAX 5 //point light ���� �ִ밪
#define NUM_AREA_LIGHT_MAX 100 //Area Light���� �ִ밪
#define MAX_RECURSION_DEPTH 20 //TraceRay��� ȣ�� depth
#define NUM_RTAO_RAY 4 // RTAO�� ����� Ray��
#define RTAO_RADIUS 0.2  // RTAO�� ������ �ݱ��� ������
#define PI 3.14159265359f // ����������
#define RANDOM_SEQUENCE_LENGTH (64 * 64) //�����ѹ� ���ۿ� ����� 0~1 ����float���� ���� ����


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

struct RandomConstantBuffer
{
    XMVECTOR randFloats0[RANDOM_SEQUENCE_LENGTH / 4];
	XMVECTOR randFloats1[RANDOM_SEQUENCE_LENGTH / 4];
};

struct PointLightConstantBuffer
{
    XMVECTOR position[NUM_POINT_LIGHT_MAX];
    XMFLOAT4 lumen[NUM_POINT_LIGHT_MAX];//(Lumen,padding,padding,padding), ���
    UINT numPointLight;//point light ����
};

struct AreaLightConstantBuffer
{
    XMVECTOR position[NUM_AREA_LIGHT_MAX];//area light�� world position
    XMVECTOR normal[NUM_AREA_LIGHT_MAX];//area light�� �ٶ󺸴� ����
    XMVECTOR vertices[NUM_AREA_LIGHT_MAX][3];//area light�� �����ϴ� 3���� vertex����

    
};

struct MeshConstantBuffer
{
    XMMATRIX world;//float(32bit)�� 16�� -> DWORD 16���ʿ�
    XMFLOAT4 albedo;//float(32bit)�� 4�� -> DWORD 4�� �ʿ�
    UINT hasAlbedoTexture;//1�̸� texture����, 0�̸� ����, 32bit�� 1�� -> DWORD 1�� �ʿ�
    UINT hasNormalTexture;//1�̸� texture����, 0�̸� ����, 32bit�� 1�� -> DWORD 1�� �ʿ�
    UINT hasSpecularTexture;//1�̸� texture����, 0�̸� ����, 32bit�� 1�� -> DWORD 1�� �ʿ�
    UINT hasRoughnessTexture;//1�̸� texture����, 0�̸� ����, 32bit�� 1�� -> DWORD 1�� �ʿ�
    UINT hasMetallicTexture;//1�̸� texture����, 0�̸� ����, 32bit�� 1�� -> DWORD 1�� �ʿ�
    FLOAT roughness;//��ģ����, 32bit�� 1�� -> DWORD 1�� �ʿ�
    FLOAT metallic;//�ݼ��� ����, 32bit�� 1�� -> DWORD 1�� �ʿ�
    FLOAT emission;//���� ���ϴ� ����, 32bit�� 1�� -> DWORD 1�� �ʿ�
};

struct Vertex
{
    XMFLOAT3 position;
    XMFLOAT2 uv;
    XMFLOAT3 normal;
    XMFLOAT3 tangent;
    XMFLOAT3 biTangent;
};
struct PathTracerRayPayload
{
    XMFLOAT4 color;
    XMFLOAT3 camera;//������ hit��Ȳ������ cameraPosition
    UINT recursionDepth;
};

struct RealTimeRayPayload
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
        RealTime = 0,   
        Shadow,
        RTAO,
        PathTracer
    };
    static const UINT Count = 4;
}

namespace TraceRayParameters
{
    static const UINT InstanceMask = ~0; 
    static const UINT HitGroupOffset[RayType::Count] = {
        0, // RealTime ray
        1, // Shadow ray
        2, // RTAO ray
        3, // PathTracer ray
    };
    static const UINT GeometryStride = RayType::Count;
    static const UINT MissShaderOffset[RayType::Count] = {
        0, // RealTime ray
        1, // Shadow ray
        2, // RTAO ray
        3, // PathTracer ray
    };
}



#endif