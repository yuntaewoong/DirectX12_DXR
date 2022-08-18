#include "pch.h"
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
        현재 HitGroup SBT은
        Renderable당 Radiance ray Hitgroup를 가리키는 Shader Record,
        Shadow Ray Hit Group을 가리키는 Shader Record가 존재함.

        구조는 아래와 같음
        record[0] == Renderable[0] radiance hit group
        record[1] == Renderable[0] shadow hit group
        record[2] == Renderable[1] radiance hit group
        record[3] == Renderable[1] shadow hit group
        record[4] == Renderable[2] radiance hit group
        record[5] == Renderable[2] shadow hit group
        ...
        .
        ....
        ======================================*/
        LocalRootArgument rootArgument = {
            .cb = {
                .world = XMMATRIX(),
                .albedo = XMFLOAT4(0.0f,1.0f,0.0f,1.0f),
                .hasTexture = 0
            },
            .vbGPUAddress = D3D12_GPU_VIRTUAL_ADDRESS(),
            .ibGPUAddress = D3D12_GPU_VIRTUAL_ADDRESS(),
            .diffuseTextureDescriptorHandle = D3D12_GPU_DESCRIPTOR_HANDLE()
        };
        for (UINT i = 0; i < renderables.size(); i++)
        {
            rootArgument.cb.world = XMMatrixTranspose(renderables[i]->GetWorldMatrix());
            rootArgument.cb.albedo = renderables[i]->GetColor();
            rootArgument.vbGPUAddress = renderables[i]->GetVertexBuffer()->GetGPUVirtualAddress();
            rootArgument.ibGPUAddress = renderables[i]->GetIndexBuffer()->GetGPUVirtualAddress();
            if (renderables[i]->GetMaterial()->HasDiffuseTexture())
            {
                rootArgument.cb.hasTexture = 1u;
                rootArgument.diffuseTextureDescriptorHandle = renderables[i]->GetMaterial()->GetDiffuseTexture()->GetDescriptorHandle();
            }
            else
            {
                rootArgument.cb.hasTexture = 0u;
            }
            for (UINT j = 0; j < RayType::Count; j++)
            {
                void* hitGroupIdentifier = stateObjectProperties->GetShaderIdentifier(HIT_GROUP_NAMES[j]);
                Push_back(ShaderRecord(hitGroupIdentifier, shaderIdentifierSize, &rootArgument, sizeof(rootArgument)));
            }
        }
        return hr;
    }
}