#include "ShaderTable\HitGroupShaderTable.h"
namespace library
{
    HitGroupShaderTable::HitGroupShaderTable() :
        ShaderTable::ShaderTable()
    {}

    HRESULT HitGroupShaderTable::Initialize(_In_ const ComPtr<ID3D12Device>& pDevice, _In_ const ComPtr<ID3D12StateObject>& pStateObject, _In_ const std::vector<std::shared_ptr<Renderable>>& renderables)
    {
        HRESULT hr = S_OK;
        UINT shaderIdentifierSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
        UINT numShaderRecords = static_cast<UINT>(renderables.size()) * RayType::Count;
        UINT shaderRecordSize = shaderIdentifierSize + sizeof(LocalRootArgument);
        hr = ShaderTable::initialize(pDevice, numShaderRecords, shaderRecordSize);
        if (FAILED(hr))
        {
            return hr;
        }
        ComPtr<ID3D12StateObjectProperties> stateObjectProperties(nullptr);
        pStateObject.As(&stateObjectProperties);

        /*=====================================
        ���� HitGroup SBT��
        Renderable�� Radiance ray Hitgroup�� ����Ű�� Shader Record,
        Shadow Ray Hit Group�� ����Ű�� Shader Record�� ������.

        ������ �Ʒ��� ����
        record[0] == Renderable[0] radiance hit group
        record[1] == Renderable[1] radiance hit group
        record[2] == Renderable[2] radiance hit group
        record[3] == Renderable[0] shadow hit group
        record[4] == Renderable[1] shadow hit group
        record[5] == Renderable[2] shadow hit group
        ======================================*/
        LocalRootArgument rootArgument = {
            .cb = {
                .albedo = XMFLOAT4(0.0f,1.0f,0.0f,1.0f)
            }
        };
        for (UINT i = 0; i < RayType::Count; i++)
        {
            for (UINT j = 0; j < renderables.size(); j++)
            {
                void* hitGroupIdentifier = stateObjectProperties->GetShaderIdentifier(HIT_GROUP_NAMES[i]);
                //rootArgument.cb.albedo = XMFLOAT4((FLOAT)(i+1)/RayType::Count, (FLOAT)(j+1)/renderables.size()*2.f, 0.f, 1.f);
                rootArgument.cb.albedo = renderables[j]->GetColor();
                Push_back(ShaderRecord(hitGroupIdentifier, shaderIdentifierSize, &rootArgument, sizeof(rootArgument)));
            }
        }
        return hr;
    }
}