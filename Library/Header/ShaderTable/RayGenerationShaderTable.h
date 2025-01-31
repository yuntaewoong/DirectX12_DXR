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

        HRESULT Initialize(
            _In_ const ComPtr<ID3D12Device>& pDevice,
            _In_ const ComPtr<ID3D12StateObject>& pStateObject,
            _In_ const LPCWSTR& raygenShaderName
        );
	};
}