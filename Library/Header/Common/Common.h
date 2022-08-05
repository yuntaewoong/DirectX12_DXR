#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef UNICODE
#define UNICODE
#endif 
#include <memory>
#include <wrl.h>
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>
#include "d3dx12.h"
#include <unordered_map>
#include <string>

#define SizeOfInUint32(obj) ((sizeof(obj) - 1) / sizeof(UINT32) + 1)

using namespace Microsoft::WRL;
using namespace DirectX;

enum class EGlobalRootSignatureSlot
{
    OutputViewSlot = 0,
    AccelerationStructureSlot,
    SceneConstantSlot,
    VertexBuffersSlot
};
const static INT NUM_OF_GLOBAL_ROOT_SIGNATURE = 4;
enum class ELocalRootSignatureSlot
{
    CubeConstantSlot = 0,
};
const static INT NUM_OF_LOCAL_ROOT_SIGNATURE = 1;

//Shader Constant Buffer
struct Viewport
{
    float left;
    float top;
    float right;
    float bottom;
};
struct RayGenConstantBuffer
{
    Viewport viewport;
    Viewport stencil;
};
struct SceneConstantBuffer
{
    XMMATRIX projectionToWorld;
    XMVECTOR cameraPosition;
    XMVECTOR lightPosition;
    XMVECTOR lightAmbientColor;
    XMVECTOR lightDiffuseColor;
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