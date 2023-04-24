#include "pch.h"
#include "RootSignature\LocalRootSignature.h"

namespace library
{
    LocalRootSignature::LocalRootSignature() :
        m_rootParameter()
    {}
    HRESULT LocalRootSignature::Initialize(_In_ ID3D12Device* pDevice)
    {
        CD3DX12_DESCRIPTOR_RANGE ranges[NUM_OF_LOCAL_ROOT_SIGNATURE_SLOT] = {};
        ranges[static_cast<UINT>(ELocalRootSignatureSlot::DiffuseTextureSlot)].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4);  // 4번 레지스터는 SRV Texture2D
        ranges[static_cast<UINT>(ELocalRootSignatureSlot::NormalTextureSlot)].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 5);  // 5번 레지스터는 SRV Texture2D
        ranges[static_cast<UINT>(ELocalRootSignatureSlot::SpecularTextureSlot)].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 6);  // 6번 레지스터는 SRV Texture2D
        ranges[static_cast<UINT>(ELocalRootSignatureSlot::RoughnessTextureSlot)].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 7);  // 7번 레지스터는 SRV Texture2D
        ranges[static_cast<UINT>(ELocalRootSignatureSlot::MetallicTextureSlot)].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 8);  // 8번 레지스터는 SRV Texture2D

        
        m_rootParameter[static_cast<UINT>(ELocalRootSignatureSlot::CubeConstantSlot)].InitAsConstants(static_cast<UINT>(sizeof(MeshConstantBuffer)) / 4u, 1u);//1번 레지스터에 MeshConstantBuffer만큼의 공간 할당
        m_rootParameter[static_cast<UINT>(ELocalRootSignatureSlot::VertexBufferSlot)].InitAsShaderResourceView(2);//2번 레지스터는 VB
        m_rootParameter[static_cast<UINT>(ELocalRootSignatureSlot::IndexBufferSlot)].InitAsShaderResourceView(3);//3번 레지스터는 IB
        m_rootParameter[static_cast<UINT>(ELocalRootSignatureSlot::DiffuseTextureSlot)].InitAsDescriptorTable(1, &ranges[static_cast<UINT>(ELocalRootSignatureSlot::DiffuseTextureSlot)]);
        m_rootParameter[static_cast<UINT>(ELocalRootSignatureSlot::NormalTextureSlot)].InitAsDescriptorTable(1, &ranges[static_cast<UINT>(ELocalRootSignatureSlot::NormalTextureSlot)]);
        m_rootParameter[static_cast<UINT>(ELocalRootSignatureSlot::SpecularTextureSlot)].InitAsDescriptorTable(1, &ranges[static_cast<UINT>(ELocalRootSignatureSlot::SpecularTextureSlot)]);
        m_rootParameter[static_cast<UINT>(ELocalRootSignatureSlot::RoughnessTextureSlot)].InitAsDescriptorTable(1, &ranges[static_cast<UINT>(ELocalRootSignatureSlot::RoughnessTextureSlot)]);
        m_rootParameter[static_cast<UINT>(ELocalRootSignatureSlot::MetallicTextureSlot)].InitAsDescriptorTable(1, &ranges[static_cast<UINT>(ELocalRootSignatureSlot::MetallicTextureSlot)]);
        D3D12_STATIC_SAMPLER_DESC staticSamplerDesc =
        {//local root signature의 sampler에 대한 정보(Root Signature안에 Sampler정보가 내장되므로 HitGroup Table에서 데이터 입력해줄 필요없음
            .Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR,
            .AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
            .AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
            .AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
            .MipLODBias = 0,
            .MaxAnisotropy = 0,
            .ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER,
            .BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK,
            .MinLOD = 0,
            .MaxLOD = D3D12_FLOAT32_MAX,
            .ShaderRegister = 0u,
            .RegisterSpace = 0u,
            .ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL
        };
        CD3DX12_ROOT_SIGNATURE_DESC localRootSignatureDesc(
            ARRAYSIZE(m_rootParameter), //local root signature의 파라미터 개수
            m_rootParameter,            //파라미터 정보
            1u,                         //static sampler개수
            &staticSamplerDesc          //static sampler정보
        );
        localRootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
        return RootSignature::initialize(pDevice, localRootSignatureDesc);
    }
}