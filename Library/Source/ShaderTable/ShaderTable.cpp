#include "Common\pch.h"
#include "ShaderTable\ShaderTable.h"

namespace library
{
    ShaderTable::ShaderTable() :  
        m_resource(nullptr),
        m_shaderRecords(),
        m_mappedShaderRecords(nullptr),
        m_shaderRecordSize(0u)
    {}
    ShaderTable::~ShaderTable()
    {
        if (m_resource.Get())
        {
            m_resource->Unmap(0, nullptr);
        }
    }
    HRESULT ShaderTable::Push_back(const ShaderRecord& shaderRecord)
    {
        if (m_shaderRecords.size() >= m_shaderRecords.capacity())
        {
            return E_FAIL;
        }
        m_shaderRecords.push_back(shaderRecord);
        shaderRecord.CopyTo(m_mappedShaderRecords);
        m_mappedShaderRecords += m_shaderRecordSize;
        return S_OK;
    }
    UINT ShaderTable::GetShaderRecordSize() const
    {
        return m_shaderRecordSize;
    }
    D3D12_GPU_VIRTUAL_ADDRESS ShaderTable::GetShaderTableGPUVirtualAddress() const
    {
        return m_resource->GetGPUVirtualAddress();
    }
    UINT64 ShaderTable::GetShaderTableSizeInBytes() const
    {
        return m_resource->GetDesc().Width;
    }
    UINT64 ShaderTable::GetShaderTableStrideInBytes() const
    {
        return m_resource->GetDesc().Width / m_shaderRecords.size();
    }
    HRESULT ShaderTable::initialize(_In_ const ComPtr<ID3D12Device>& pDevice, _In_ UINT numShaderRecords, _In_ UINT shaderRecordSize)
    {
        HRESULT hr = S_OK;
        m_shaderRecordSize = align(shaderRecordSize, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);//32바이트 단위로
        m_shaderRecords.reserve(numShaderRecords);
        UINT bufferSize = numShaderRecords * m_shaderRecordSize;//buffer사이즈 설정

        CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
        CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
        hr = pDevice->CreateCommittedResource(
            &uploadHeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_resource)
        );
        if (FAILED(hr))
        {
            return hr;
        }
        hr = mapCpuWriteOnly();
        if (FAILED(hr))
        {
            return hr;
        }
        return hr;
    }
    UINT ShaderTable::align(_In_ UINT size, _In_ UINT alignment)
    {
        return (size + (alignment - 1)) & ~(alignment - 1);
    }
    HRESULT ShaderTable::mapCpuWriteOnly()
    {
        HRESULT hr = S_OK;
        uint8_t* mappedData(nullptr);
        CD3DX12_RANGE readRange(0, 0);
        hr = m_resource->Map(0, &readRange, reinterpret_cast<void**>(&mappedData));
        if (FAILED(hr))
        {
            return hr;
        }
        m_mappedShaderRecords = mappedData;
        return hr;
    }
}