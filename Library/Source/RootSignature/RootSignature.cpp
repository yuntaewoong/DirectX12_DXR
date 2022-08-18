#include "pch.h"
#include "RootSignature\RootSignature.h"

namespace library
{
    RootSignature::RootSignature() :
        m_rootSignature(nullptr)
    {}
    ComPtr<ID3D12RootSignature>& RootSignature::GetRootSignature()
    {
        return m_rootSignature;
    }
    HRESULT RootSignature::initialize(_In_ ID3D12Device* pDevice,_In_ CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc)
    {
        HRESULT hr = S_OK;
        ComPtr<ID3DBlob> signature(nullptr);
        ComPtr<ID3DBlob> error(nullptr);
        hr = D3D12SerializeRootSignature(
            &rootSignatureDesc,
            D3D_ROOT_SIGNATURE_VERSION_1,
            &signature,
            &error
        );// 루트 시그니처의 binary화
        if (FAILED(hr))
        {
            return hr;
        }
        hr = pDevice->CreateRootSignature(
            1, 
            signature->GetBufferPointer(),
            signature->GetBufferSize(),
            IID_PPV_ARGS(&m_rootSignature)
        );//루트 시그니처 생성
        if (FAILED(hr))
        {
            return hr;
        }
        return S_OK;
    }
}