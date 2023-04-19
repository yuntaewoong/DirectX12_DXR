#include "pch.h"
#include "ShaderTable\HitGroupShaderTable.h"
namespace library
{
    HitGroupShaderTable::HitGroupShaderTable() :
        ShaderTable::ShaderTable()
    {}

    HRESULT HitGroupShaderTable::Initialize(_In_ const ComPtr<ID3D12Device>& pDevice, _In_ const ComPtr<ID3D12StateObject>& pStateObject, _In_ const std::vector<std::shared_ptr<Mesh>>& meshes)
    {
        HRESULT hr = S_OK;
        UINT shaderIdentifierSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
        UINT numShaderRecords = static_cast<UINT>(meshes.size()) * RayType::Count;
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
        record[0] == Meshes[0] radiance hit group
        record[1] == Meshes[0] shadow hit group
        record[2] == Meshes[0] RTAO hit group
        record[3] == Meshes[1] radiance hit group
        record[4] == Meshes[1] shadow hit group
        record[5] == Meshes[1] RTAO hit group
        ...
        .
        ....
        ======================================*/
        LocalRootArgument rootArgument = {
            .cb = {
                .world = XMMATRIX(),
                .albedo = XMFLOAT4(0.0f,1.0f,0.0f,1.0f),
                .hasTexture = 0,
                .reflectivity = 0.f
            },
            .vbGPUAddress = D3D12_GPU_VIRTUAL_ADDRESS(),
            .ibGPUAddress = D3D12_GPU_VIRTUAL_ADDRESS(),
            .diffuseTextureDescriptorHandle = D3D12_GPU_DESCRIPTOR_HANDLE()
        };
        for (UINT i = 0; i < meshes.size(); i++)
        {
            rootArgument.cb.world = XMMatrixTranspose(meshes[i]->GetWorldMatrix());
            rootArgument.cb.albedo = meshes[i]->GetColor();
            rootArgument.cb.reflectivity = meshes[i]->GetMaterial()->GetReflectivity();
            rootArgument.vbGPUAddress = meshes[i]->GetVertexBuffer()->GetGPUVirtualAddress();
            rootArgument.ibGPUAddress = meshes[i]->GetIndexBuffer()->GetGPUVirtualAddress();
            rootArgument.cb.hasTexture = 0u;
            if (meshes[i]->GetMaterial()->HasDiffuseTexture())
            {
                rootArgument.cb.hasTexture = 1u;
                rootArgument.diffuseTextureDescriptorHandle = meshes[i]->GetMaterial()->GetDiffuseTexture()->GetDescriptorHandle();
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