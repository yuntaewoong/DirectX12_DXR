#pragma once
#include "ShaderTable\ShaderTable.h"

namespace library
{
    /*
    *   Miss Shader Table Wrapper
    */
    class MissShaderTable final : public ShaderTable
    {
    public:
        MissShaderTable();
        MissShaderTable(const MissShaderTable& other) = delete;
        MissShaderTable(MissShaderTable&& other) = delete;
        MissShaderTable& operator=(const MissShaderTable& other) = delete;
        MissShaderTable& operator=(MissShaderTable&& other) = delete;

        virtual HRESULT Initialize(_In_ ID3D12Device* pDevice, _In_ ComPtr<ID3D12StateObject>& pStateObject) override;
    };
}