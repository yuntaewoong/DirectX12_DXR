#pragma once
#include <DirectXMath.h>
#define NUM_LIGHT 2
using namespace DirectX;
/*
    Shader���� ���Ǵ� struct�� type����
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