#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef UNICODE
#define UNICODE
#endif 

#include "pch.h"
#include "DataTypeSharedBothHLSLAndCPP.h"

using namespace Microsoft::WRL;
using namespace DirectX;
namespace library
{
    const static UINT NUM_DESCRIPTORS_DEFAULT = 50u;

    /*===========================================
    Shader 진입점,
    Hit Group 진입점 정의
    =====================================*/
    constexpr LPCWSTR RAY_GEN_SHADER_NAME = L"MyRaygenShader";
    constexpr LPCWSTR CLOSEST_HIT_SHADER_NAMES[RayType::Count] =
    {
        L"MyClosestHitShader",
        L"MyShadowRayClosestHitShader",
        L"RTAORayClosestHitShader"
    };
    constexpr LPCWSTR MISS_SHADER_NAMES[RayType::Count] =
    {
        L"MyMissShader",
        L"MyShadowRayMissShader",
        L"RTAORayMissShader"
    };
    constexpr LPCWSTR HIT_GROUP_NAMES[RayType::Count] =
    {
        L"MyHitGroup", 
        L"MyShadowRayHitGroup",
        L"RTAOHitGroup"
    };

    /*==========================================
    Global Root Signature Slot
    Local Root Signature Slot상수 정의
    ======================================*/
    enum class EGlobalRootSignatureSlot
    {
        OutputViewSlot = 0,
        AccelerationStructureSlot,
        CameraConstantSlot,
        LightConstantSlot
    };
    const static INT NUM_OF_GLOBAL_ROOT_SIGNATURE_SLOT = 6;
    enum class ELocalRootSignatureSlot
    {
        CubeConstantSlot = 0,
        VertexBufferSlot = 1,
        IndexBufferSlot = 2,
        DiffuseTextureSlot = 3,
        NormalTextureSlot = 4,
        SpecularTextureSlot = 5
    };
    const static INT NUM_OF_LOCAL_ROOT_SIGNATURE_SLOT = 6;
    struct LocalRootArgument
    {
        MeshConstantBuffer cb;//Root Constant
        D3D12_GPU_VIRTUAL_ADDRESS vbGPUAddress;//Inline Descriptor
        D3D12_GPU_VIRTUAL_ADDRESS ibGPUAddress;//Inline Descriptor
        D3D12_GPU_DESCRIPTOR_HANDLE diffuseTextureDescriptorHandle;//Descriptor Table
        D3D12_GPU_DESCRIPTOR_HANDLE normalTextureDescriptorHandle;//Descriptor Table
        D3D12_GPU_DESCRIPTOR_HANDLE specularTextureDescriptorHandle;//Descriptor Table
    };


    /*=============
    키보드 인풋 정의
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
    마우스 인풋 정의
    ======*/
    struct MouseRelativeMovement
    {
        LONG X;
        LONG Y;
    };
}