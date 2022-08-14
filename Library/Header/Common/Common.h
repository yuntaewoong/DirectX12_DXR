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
#include <string>
#include <algorithm>
#include "DataTypeSharedBothHLSLAndCPP.h"

using namespace Microsoft::WRL;
using namespace DirectX;
namespace library
{
    /*===========================================
    Shader ������,
    Hit Group ������ ����
    =====================================*/
    constexpr LPCWSTR RAY_GEN_SHADER_NAME = L"MyRaygenShader";
    constexpr LPCWSTR CLOSEST_HIT_SHADER_NAMES[RayType::Count] =
    {
        L"MyClosestHitShader",
        L"MyShadowRayClosestHitShader",
    };
    constexpr LPCWSTR MISS_SHADER_NAMES[RayType::Count] =
    {
        L"MyMissShader",
        L"MyShadowRayMissShader"
    };
    constexpr LPCWSTR HIT_GROUP_NAMES[RayType::Count] =
    {
        L"MyHitGroup", 
        L"MyShadowRayHitGroup"
    };

    /*==========================================
    Global Root Signature Slot
    Local Root Signature Slot��� ����
    ======================================*/
    enum class EGlobalRootSignatureSlot
    {
        OutputViewSlot = 0,
        AccelerationStructureSlot,
        CameraConstantSlot,
        LightConstantSlot
    };
    const static INT NUM_OF_GLOBAL_ROOT_SIGNATURE_SLOT = 4;
    enum class ELocalRootSignatureSlot
    {
        CubeConstantSlot = 0,
        VertexBufferSlot = 1,
        IndexBufferSlot = 2
    };
    const static INT NUM_OF_LOCAL_ROOT_SIGNATURE_SLOT = 3;
    struct LocalRootArgument
    {
        CubeConstantBuffer cb;
        D3D12_GPU_VIRTUAL_ADDRESS vbGPUAddress;
        D3D12_GPU_VIRTUAL_ADDRESS ibGPUAddress;
    };


    /*=============
    Ű���� ��ǲ ����
    ==========*/
    struct DirectionsInput
    {
        BOOL bFront;
        BOOL bLeft;
        BOOL bBack;
        BOOL bRight;
        BOOL bUp;
        BOOL bDown;
    };

    /*======
    ���콺 ��ǲ ����
    ======*/
    struct MouseRelativeMovement
    {
        LONG X;
        LONG Y;
    };
}