#pragma once
#include "ShaderTable\ShaderTable.h"
#include "Common\ShaderDataType.h"
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

        virtual HRESULT Initialize(_In_ ID3D12Device* pDevice, _In_ ComPtr<ID3D12StateObject>& pStateObject) override;
    };
}