#include "RootSignature\GlobalRootSignature.h"

namespace library
{
    GlobalRootSignature::GlobalRootSignature() :
        m_rootParameter()
    {}
    HRESULT GlobalRootSignature::Initialize(_In_ ID3D12Device* pDevice)
    {
        HRESULT hr = S_OK;

        CD3DX12_DESCRIPTOR_RANGE ranges[2] = {};
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);  // 0번 레지스터는 Output UAV텍스처
        ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 1);  // 1번부터 2개의 레지스터는 Vertex,Index버퍼

        m_rootParameter[static_cast<UINT>(EGlobalRootSignatureSlot::OutputViewSlot)].InitAsDescriptorTable(1, &ranges[0]);//ranges[0]정보로 초기화
        m_rootParameter[static_cast<UINT>(EGlobalRootSignatureSlot::AccelerationStructureSlot)].InitAsShaderResourceView(0);//t0번 레지스터는 AS다
        m_rootParameter[static_cast<UINT>(EGlobalRootSignatureSlot::CameraConstantSlot)].InitAsConstantBufferView(0);//b0번 레지스터는 Camera Constant Buffer다
        m_rootParameter[static_cast<UINT>(EGlobalRootSignatureSlot::LightConstantSlot)].InitAsConstantBufferView(2);//b3번 레지스터는 Light Constant Buffer다
        m_rootParameter[static_cast<UINT>(EGlobalRootSignatureSlot::VertexBuffersSlot)].InitAsDescriptorTable(1, &ranges[1]);//ranges[1]정보로 초기화

        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(ARRAYSIZE(m_rootParameter), m_rootParameter);

        hr = createRootSignature(pDevice, rootSignatureDesc);
        if (FAILED(hr))
        {
            return hr;
        }
        return hr;
    }
}