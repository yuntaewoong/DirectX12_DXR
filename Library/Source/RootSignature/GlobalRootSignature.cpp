#include "pch.h"
#include "RootSignature\GlobalRootSignature.h"

namespace library
{
    GlobalRootSignature::GlobalRootSignature() :
        m_rootParameter()
    {}
    HRESULT GlobalRootSignature::Initialize(_In_ ID3D12Device* pDevice)
    {   
        CD3DX12_DESCRIPTOR_RANGE ranges[1] = {};
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);  // 0번 레지스터는 Output UAV텍스처

        m_rootParameter[static_cast<UINT>(EGlobalRootSignatureSlot::OutputViewSlot)].InitAsDescriptorTable(1, &ranges[0]);//ranges[0]정보로 초기화
        m_rootParameter[static_cast<UINT>(EGlobalRootSignatureSlot::AccelerationStructureSlot)].InitAsShaderResourceView(0);//t0번 레지스터는 AS다
        m_rootParameter[static_cast<UINT>(EGlobalRootSignatureSlot::CameraConstantSlot)].InitAsConstantBufferView(0);//b0번 레지스터는 Camera Constant Buffer다
        m_rootParameter[static_cast<UINT>(EGlobalRootSignatureSlot::LightConstantSlot)].InitAsConstantBufferView(2);//b2번 레지스터는 Light Constant Buffer다
        m_rootParameter[static_cast<UINT>(EGlobalRootSignatureSlot::RandomConstantSlot)].InitAsConstantBufferView(7);//b7번 레지스터는 Random Constant Buffer다
        


        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(ARRAYSIZE(m_rootParameter), m_rootParameter);

        return RootSignature::initialize(pDevice, rootSignatureDesc);;
    }
}