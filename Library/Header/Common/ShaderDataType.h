#pragma once
#include "Common\Common.h"
#define NUM_LIGHT 2

/*
    Shader���� ���Ǵ� struct�� type����
*/
namespace library
{
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