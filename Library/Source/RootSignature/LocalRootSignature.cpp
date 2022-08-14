#include "RootSignature\LocalRootSignature.h"

namespace library
{
    LocalRootSignature::LocalRootSignature() :
        m_rootParameter()
    {}
    HRESULT LocalRootSignature::Initialize(_In_ ID3D12Device* pDevice)
    {
        m_rootParameter[static_cast<UINT>(ELocalRootSignatureSlot::CubeConstantSlot)].InitAsConstants(4u, 1u);//1�� �������Ϳ� 32��Ʈ(4����Ʈ) �� 4���� ���ڴ�
        m_rootParameter[static_cast<UINT>(ELocalRootSignatureSlot::VertexBufferSlot)].InitAsShaderResourceView(2);//2�� �������ʹ� VB
        m_rootParameter[static_cast<UINT>(ELocalRootSignatureSlot::IndexBufferSlot)].InitAsShaderResourceView(3);//3�� �������ʹ� IB

        CD3DX12_ROOT_SIGNATURE_DESC localRootSignatureDesc(ARRAYSIZE(m_rootParameter), m_rootParameter);
        localRootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
        return RootSignature::initialize(pDevice, localRootSignatureDesc);
    }
}