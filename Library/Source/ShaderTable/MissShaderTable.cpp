#include "pch.h"
#include "ShaderTable\MissShaderTable.h"
namespace library
{
    MissShaderTable::MissShaderTable() :
        ShaderTable::ShaderTable()
    {}
    HRESULT MissShaderTable::Initialize(_In_ const ComPtr<ID3D12Device>& pDevice, _In_ const ComPtr<ID3D12StateObject>& pStateObject)
    {
        HRESULT hr = S_OK;
        UINT shaderIdentifierSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
        UINT numShaderRecords = RayType::Count;//Miss Shader는 ray type별로 존재
        UINT shaderRecordSize = shaderIdentifierSize;
        hr = ShaderTable::initialize(pDevice, numShaderRecords, shaderRecordSize);
        if (FAILED(hr))
        {
            return hr;
        }
        ComPtr<ID3D12StateObjectProperties> stateObjectProperties(nullptr);
        pStateObject.As(&stateObjectProperties);
        for (UINT i = 0; i < RayType::Count; i++)
        {
            void* missShaderIdentifier = stateObjectProperties->GetShaderIdentifier(MISS_SHADER_NAMES[i]);
            Push_back(ShaderRecord(missShaderIdentifier, shaderIdentifierSize));
        }
        return hr;
    }
}