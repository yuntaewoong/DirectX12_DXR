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
    enum class EGlobalRootSignatureSlot
    {
        OutputViewSlot = 0,
        AccelerationStructureSlot,
        CameraConstantSlot,
        LightConstantSlot,
        VertexBuffersSlot
    };
    const static INT NUM_OF_GLOBAL_ROOT_SIGNATURE_SLOT = 5;
    enum class ELocalRootSignatureSlot
    {
        CubeConstantSlot = 0,
    };
    const static INT NUM_OF_LOCAL_ROOT_SIGNATURE_SLOT = 1;
    struct LocalRootArgument
    {
        CubeConstantBuffer cb;
    };

    struct DirectionsInput
    {
        BOOL bFront;
        BOOL bLeft;
        BOOL bBack;
        BOOL bRight;
        BOOL bUp;
        BOOL bDown;
    };
    struct MouseRelativeMovement
    {
        LONG X;
        LONG Y;
    };
}