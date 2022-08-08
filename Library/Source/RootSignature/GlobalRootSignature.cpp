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
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);  // 0�� �������ʹ� Output UAV�ؽ�ó
        ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 1);  // 1������ 2���� �������ʹ� Vertex,Index����

        m_rootParameter[static_cast<UINT>(EGlobalRootSignatureSlot::OutputViewSlot)].InitAsDescriptorTable(1, &ranges[0]);//ranges[0]������ �ʱ�ȭ
        m_rootParameter[static_cast<UINT>(EGlobalRootSignatureSlot::AccelerationStructureSlot)].InitAsShaderResourceView(0);//t0�� �������ʹ� AS��
        m_rootParameter[static_cast<UINT>(EGlobalRootSignatureSlot::CameraConstantSlot)].InitAsConstantBufferView(0);//b0�� �������ʹ� Camera Constant Buffer��
        m_rootParameter[static_cast<UINT>(EGlobalRootSignatureSlot::LightConstantSlot)].InitAsConstantBufferView(2);//b3�� �������ʹ� Light Constant Buffer��
        m_rootParameter[static_cast<UINT>(EGlobalRootSignatureSlot::VertexBuffersSlot)].InitAsDescriptorTable(1, &ranges[1]);//ranges[1]������ �ʱ�ȭ

        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(ARRAYSIZE(m_rootParameter), m_rootParameter);

        hr = createRootSignature(pDevice, rootSignatureDesc);
        if (FAILED(hr))
        {
            return hr;
        }
        return hr;
    }
}