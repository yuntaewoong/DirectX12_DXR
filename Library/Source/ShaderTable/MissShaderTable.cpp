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
        UINT numShaderRecords = 2;
        UINT shaderRecordSize = shaderIdentifierSize;
        hr = ShaderTable::initialize(pDevice, numShaderRecords, shaderRecordSize);
        if (FAILED(hr))
        {
            return hr;
        }
        ComPtr<ID3D12StateObjectProperties> stateObjectProperties(nullptr);
        pStateObject.As(&stateObjectProperties);

        void* missShaderIdentifier = stateObjectProperties->GetShaderIdentifier(L"MyMissShader");
        Push_back(ShaderRecord(missShaderIdentifier, shaderIdentifierSize));
        void* shadowRayMissShaderIdentifier = stateObjectProperties->GetShaderIdentifier(L"MyShadowRayMissShader");
        Push_back(ShaderRecord(shadowRayMissShaderIdentifier, shaderIdentifierSize));

        return hr;
    }
}