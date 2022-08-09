#pragma once
#include "ShaderTable\ShaderTable.h"

namespace library
{
    /*
    *   RayGenerationShader Table Wrapper
    */
	class RayGenerationShaderTable final : public ShaderTable
	{
    public:
        RayGenerationShaderTable();
        RayGenerationShaderTable(const RayGenerationShaderTable& other) = delete;
        RayGenerationShaderTable(RayGenerationShaderTable&& other) = delete;
        RayGenerationShaderTable& operator=(const RayGenerationShaderTable& other) = delete;
        RayGenerationShaderTable& operator=(RayGenerationShaderTable&& other) = delete;

        virtual HRESULT Initialize(_In_ ID3D12Device* pDevice, _In_ ComPtr<ID3D12StateObject>& pStateObject) override;
	};
}