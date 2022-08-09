#pragma once
#include <DirectXMath.h>
#define NUM_LIGHT 2
using namespace DirectX;
/*
    Shader에서 사용되는 struct들 type정의
*/
namespace library
{
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
}