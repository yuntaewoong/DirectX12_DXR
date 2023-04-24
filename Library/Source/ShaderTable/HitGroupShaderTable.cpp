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
        Mesh당 Radiance ray Hitgroup를 가리키는 Shader Record,
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
                .hasDiffuseTexture = 0,
                .hasNormalTexture = 0,
                .hasSpecularTexture = 0,
                .reflectivity = 0.f,
                .roughness = 0.5f,
                .metallic = 0.f
            },
            .vbGPUAddress = D3D12_GPU_VIRTUAL_ADDRESS(),
            .ibGPUAddress = D3D12_GPU_VIRTUAL_ADDRESS(),
            .diffuseTextureDescriptorHandle = D3D12_GPU_DESCRIPTOR_HANDLE(),
            .normalTextureDescriptorHandle = D3D12_GPU_DESCRIPTOR_HANDLE(),
            .specularTextureDescriptorHandle = D3D12_GPU_DESCRIPTOR_HANDLE()
        };
        for (UINT i = 0; i < meshes.size(); i++)
        {
            rootArgument.cb.world = XMMatrixTranspose(meshes[i]->GetWorldMatrix());
            rootArgument.cb.albedo = meshes[i]->GetColor();
            rootArgument.cb.reflectivity = meshes[i]->GetMaterial()->GetReflectivity();
            rootArgument.cb.roughness = meshes[i]->GetMaterial()->GetRoughness();
            rootArgument.cb.metallic = meshes[i]->GetMaterial()->GetMetallic();
            rootArgument.vbGPUAddress = meshes[i]->GetVertexBuffer()->GetGPUVirtualAddress();
            rootArgument.ibGPUAddress = meshes[i]->GetIndexBuffer()->GetGPUVirtualAddress();
            rootArgument.cb.hasDiffuseTexture = 0u;
            rootArgument.cb.hasNormalTexture = 0u;
            rootArgument.cb.hasSpecularTexture = 0u;
            rootArgument.cb.hasRoughnessTexture = 0u;
            rootArgument.cb.hasMetallicTexture = 0u;
            if (meshes[i]->GetMaterial()->HasDiffuseTexture())
            {
                rootArgument.cb.hasDiffuseTexture = 1u;
                rootArgument.diffuseTextureDescriptorHandle = meshes[i]->GetMaterial()->GetDiffuseTexture()->GetDescriptorHandle();
            }
            if (meshes[i]->GetMaterial()->HasNormalTexture())
            {
                rootArgument.cb.hasNormalTexture = 1u;
                rootArgument.normalTextureDescriptorHandle = meshes[i]->GetMaterial()->GetNormalTexture()->GetDescriptorHandle();
            }
            if (meshes[i]->GetMaterial()->HasSpecularTexture())
            {
                rootArgument.cb.hasSpecularTexture = 1u;
                rootArgument.specularTextureDescriptorHandle = meshes[i]->GetMaterial()->GetSpecularTexture()->GetDescriptorHandle();
            }
            if (meshes[i]->GetMaterial()->HasRoughnessTexture())
            {
                rootArgument.cb.hasRoughnessTexture = 1u;
                rootArgument.roughnessTextureDescriptorHandle = meshes[i]->GetMaterial()->GetRoughnessTexture()->GetDescriptorHandle();
            }
            if (meshes[i]->GetMaterial()->HasMetallicTexture())
            {
                rootArgument.cb.hasMetallicTexture = 1u;
                rootArgument.metallicTextureDescriptorHandle = meshes[i]->GetMaterial()->GetMetallicTexture()->GetDescriptorHandle();
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