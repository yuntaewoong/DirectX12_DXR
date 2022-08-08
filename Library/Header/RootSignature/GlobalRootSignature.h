#pragma once

#include "Common/Common.h"
#include "RootSignature\RootSignature.h"

namespace library
{
    /*
        Global Root Signature
    */
    class GlobalRootSignature final : public library::RootSignature
    {
    public:
        GlobalRootSignature();
        GlobalRootSignature(const GlobalRootSignature& other) = delete;
        GlobalRootSignature(GlobalRootSignature&& other) = delete;
        GlobalRootSignature& operator=(const GlobalRootSignature& other) = delete;
        GlobalRootSignature& operator=(GlobalRootSignature&& other) = delete;
        virtual ~GlobalRootSignature() = default;
        virtual HRESULT Initialize(_In_ ID3D12Device* pDevice) override;
    private:
        CD3DX12_ROOT_PARAMETER m_rootParameter[NUM_OF_GLOBAL_ROOT_SIGNATURE_SLOT];
    };
}