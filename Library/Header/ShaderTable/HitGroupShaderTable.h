#pragma once
#include "ShaderTable\ShaderTable.h"
#include "Render\Mesh.h"
namespace library
{
    /*
    *   HitGroup Shader Table Wrapper
    */
    class HitGroupShaderTable final : public ShaderTable
    {
    public:
        HitGroupShaderTable();
        HitGroupShaderTable(const HitGroupShaderTable& other) = delete;
        HitGroupShaderTable(HitGroupShaderTable&& other) = delete;
        HitGroupShaderTable& operator=(const HitGroupShaderTable& other) = delete;
        HitGroupShaderTable& operator=(HitGroupShaderTable&& other) = delete;

        HRESULT Initialize(_In_ const ComPtr<ID3D12Device>& pDevice, _In_ const ComPtr<ID3D12StateObject>& pStateObject,_In_ const std::vector<std::shared_ptr<Mesh>>& meshes);
    private:
    };
}