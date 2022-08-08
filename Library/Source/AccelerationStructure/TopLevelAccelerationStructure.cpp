#include "AccelerationStructure\TopLevelAccelerationStructure.h"

namespace library
{
	TopLevelAccelerationStructure::TopLevelAccelerationStructure() :
        AccelerationStructure::AccelerationStructure(),
        m_instanceResource(nullptr),
        m_instanceDescs(std::vector<D3D12_RAYTRACING_INSTANCE_DESC>())
	{}
    HRESULT TopLevelAccelerationStructure::Initialize(
        _In_ ID3D12Device5* pDevice,
        _In_ ID3D12GraphicsCommandList4* pCommandList,
        _In_ const std::vector<std::unique_ptr<BottomLevelAccelerationStructure>>& m_bottomLevelAccelerationStructures
    )
    {
        HRESULT hr = S_OK;
        hr = createInstanceBuffer(pDevice, m_bottomLevelAccelerationStructures);
        if (FAILED(hr))
        {
            return hr;
        }
        hr = AccelerationStructure::initialize(pDevice, pCommandList);
        if (FAILED(hr))
        {
            return hr;
        }
        return S_OK;
	}
	void TopLevelAccelerationStructure::Update(_In_ FLOAT deltaTime)
	{}
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS TopLevelAccelerationStructure::createInput()
    {
        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS tlasInputs = {
            .Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL,//TLAS
            .Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE,
            .NumDescs = static_cast<UINT>(m_instanceDescs.size()),
            .DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY,
            .InstanceDescs = m_instanceResource->GetGPUVirtualAddress()
        };
        return tlasInputs;
    }
    HRESULT TopLevelAccelerationStructure::createInstanceBuffer(
        _In_ ID3D12Device* pDevice,
        _In_ const std::vector<std::unique_ptr<BottomLevelAccelerationStructure>>& m_bottomLevelAccelerationStructures
    )
    {
        HRESULT hr = S_OK;
        m_instanceDescs.resize(m_bottomLevelAccelerationStructures.size());
        for (INT i = 0; i < m_instanceDescs.size(); i++)
        {
            XMFLOAT4X4 transform;
            XMStoreFloat4x4(&transform, XMMatrixTranspose(m_bottomLevelAccelerationStructures[i]->GetRenderable()->GetWorldMatrix()));
            m_instanceDescs[i] = {
                .Transform = {
                    transform._11,transform._12,transform._13,transform._14,
                    transform._21,transform._22,transform._23,transform._24,
                    transform._31,transform._32,transform._33,transform._34,
                },
                .InstanceID = static_cast<UINT>(i),
                .InstanceMask = 1,
                .InstanceContributionToHitGroupIndex = static_cast<UINT>(i),
                .AccelerationStructure = m_bottomLevelAccelerationStructures[i]->GetAccelerationStructure   ()->GetGPUVirtualAddress()
            };
        }
        CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
        CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(m_instanceDescs[0]) * m_instanceDescs.size());
        hr = pDevice->CreateCommittedResource(
            &uploadHeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_instanceResource)
        );
        if (FAILED(hr))
        {
            return hr;
        }
        void* pMappedData = nullptr;
        m_instanceResource->Map(0, nullptr, &pMappedData);
        memcpy(pMappedData, m_instanceDescs.data(), m_instanceDescs.size() * sizeof(m_instanceDescs[0]));
        m_instanceResource->Unmap(0, nullptr);
        return hr;
    }
}