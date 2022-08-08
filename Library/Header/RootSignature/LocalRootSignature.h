#pragma once

#include "Common/Common.h"
#include "RootSignature\RootSignature.h"

namespace library
{
    /*
        Local Root Signature
    */
    class LocalRootSignature final : public library::RootSignature
    {
    public:
        LocalRootSignature();
        LocalRootSignature(const LocalRootSignature& other) = delete;
        LocalRootSignature(LocalRootSignature&& other) = delete;
        LocalRootSignature& operator=(const LocalRootSignature& other) = delete;
        LocalRootSignature& operator=(LocalRootSignature&& other) = delete;
        virtual ~LocalRootSignature() = default;
        virtual HRESULT Initialize(_In_ ID3D12Device* pDevice) override;
    private:
        CD3DX12_ROOT_PARAMETER m_rootParameter[NUM_OF_LOCAL_ROOT_SIGNATURE_SLOT];
    };
}