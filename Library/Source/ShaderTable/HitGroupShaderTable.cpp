#include "ShaderTable\HitGroupShaderTable.h"
namespace library
{
    HitGroupShaderTable::HitGroupShaderTable() :
        ShaderTable::ShaderTable()
    {}

    HRESULT HitGroupShaderTable::Initialize(_In_ ID3D12Device* pDevice, _In_ ComPtr<ID3D12StateObject>& pStateObject)
    {
        HRESULT hr = S_OK;
        UINT shaderIdentifierSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
        UINT numShaderRecords = 6;
        UINT shaderRecordSize = shaderIdentifierSize + sizeof(LocalRootArgument);
        hr = ShaderTable::initialize(pDevice, numShaderRecords, shaderRecordSize);
        if (FAILED(hr))
        {
            return hr;
        }
        ComPtr<ID3D12StateObjectProperties> stateObjectProperties(nullptr);
        pStateObject.As(&stateObjectProperties);
        void* hitGroupIdentifier = stateObjectProperties->GetShaderIdentifier(L"MyHitGroup");
        void* shadowRayHitGroupIdentifier = stateObjectProperties->GetShaderIdentifier(L"MyShadowRayHitGroup");
        LocalRootArgument rootArgument = {
            .cb = {
                .albedo = XMFLOAT4(0.0f,1.0f,0.0f,1.0f)
            }
        };
        


        Push_back(ShaderRecord(hitGroupIdentifier, shaderIdentifierSize,&rootArgument,sizeof(rootArgument)));
        

        rootArgument.cb.albedo = XMFLOAT4(1.f, 0.f, 0.f, 1.f);
        Push_back(ShaderRecord(hitGroupIdentifier, shaderIdentifierSize, &rootArgument, sizeof(rootArgument)));
        

        rootArgument.cb.albedo = XMFLOAT4(0.f, 1.f, 1.f, 1.f);
        Push_back(ShaderRecord(hitGroupIdentifier, shaderIdentifierSize, &rootArgument, sizeof(rootArgument)));


        Push_back(ShaderRecord(shadowRayHitGroupIdentifier, shaderIdentifierSize, &rootArgument, sizeof(rootArgument)));
        Push_back(ShaderRecord(shadowRayHitGroupIdentifier, shaderIdentifierSize, &rootArgument, sizeof(rootArgument)));
        Push_back(ShaderRecord(shadowRayHitGroupIdentifier, shaderIdentifierSize, &rootArgument, sizeof(rootArgument)));
        

        return hr;
    }
}