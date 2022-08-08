#pragma once

#include "Common/Common.h"

namespace library
{
    /*
        RootSignature의 Base 클래스
    */
    class RootSignature
    {
    public:
        RootSignature();
        RootSignature(const RootSignature& other) = delete;
        RootSignature(RootSignature&& other) = delete;
        RootSignature& operator=(const RootSignature& other) = delete;
        RootSignature& operator=(RootSignature&& other) = delete;
        ~RootSignature() = default;
        virtual HRESULT Initialize(_In_ ID3D12Device* pDevice) = 0;
        ComPtr<ID3D12RootSignature>& GetRootSignature();
    protected:
        HRESULT createRootSignature(_In_ ID3D12Device* pDevice,_In_ CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc);
    private:
        ComPtr<ID3D12RootSignature> m_rootSignature;

    };
}