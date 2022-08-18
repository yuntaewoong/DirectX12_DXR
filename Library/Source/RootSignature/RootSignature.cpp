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
        );// ��Ʈ �ñ״�ó�� binaryȭ
        if (FAILED(hr))
        {
            return hr;
        }
        hr = pDevice->CreateRootSignature(
            1, 
            signature->GetBufferPointer(),
            signature->GetBufferSize(),
            IID_PPV_ARGS(&m_rootSignature)
        );//��Ʈ �ñ״�ó ����
        if (FAILED(hr))
        {
            return hr;
        }
        return S_OK;
    }
}