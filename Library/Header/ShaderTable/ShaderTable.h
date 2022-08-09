#pragma once
#include "Common\Common.h"
#include "ShaderTable\ShaderRecord.h"
namespace library
{
    class ShaderTable
    {
    public:
        ShaderTable();
        ShaderTable(const ShaderTable& other) = delete;
        ShaderTable(ShaderTable&& other) = delete;
        ShaderTable& operator=(const ShaderTable& other) = delete;
        ShaderTable& operator=(ShaderTable&& other) = delete;
        ~ShaderTable();
        virtual HRESULT Initialize(_In_ ID3D12Device* pDevice, _In_ ComPtr<ID3D12StateObject>& pStateObject) = 0;
        HRESULT Push_back(const ShaderRecord& shaderRecord);
        UINT GetShaderRecordSize() const;
        ComPtr<ID3D12Resource>& GetResource();
    protected:
        HRESULT initialize(_In_ ID3D12Device* pDevice, _In_ UINT numShaderRecords, _In_ UINT shaderRecordSize);
    private:
        UINT align(_In_ UINT size,_In_ UINT alignment);
        HRESULT mapCpuWriteOnly();
    private:
        ComPtr<ID3D12Resource> m_resource;
        std::vector<ShaderRecord> m_shaderRecords;
        uint8_t* m_mappedShaderRecords;
        UINT m_shaderRecordSize;
    };
}