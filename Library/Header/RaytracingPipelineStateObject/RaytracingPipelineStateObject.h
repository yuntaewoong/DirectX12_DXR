#pragma once

#include "Common/Common.h"

namespace library
{
    /*
        RaytracingPipelineStateObject
    */
    class RaytracingPipelineStateObject
    {
    public:
        RaytracingPipelineStateObject();
        RaytracingPipelineStateObject(const RaytracingPipelineStateObject& other) = delete;
        RaytracingPipelineStateObject(RaytracingPipelineStateObject&& other) = delete;
        RaytracingPipelineStateObject& operator=(const RaytracingPipelineStateObject& other) = delete;
        RaytracingPipelineStateObject& operator=(RaytracingPipelineStateObject&& other) = delete;
        ~RaytracingPipelineStateObject() = default;
        HRESULT Initialize(
            _In_ const ComPtr<ID3D12Device5>& pDevice,
            _In_ const ComPtr<ID3D12RootSignature>& pLocalRootSignature,
            _In_ const ComPtr<ID3D12RootSignature>& pGlobalRootSignature
        );
        ComPtr<ID3D12StateObject>& GetStateObject();
    private:
        void createDXILSubobject();
        void createHitGroupSubobject();
        void createShaderConfigSubobject();
        void createLocalRootSignatureSubobject(_In_ const ComPtr<ID3D12RootSignature>& pLocalRootSignature);
        void createGlobalRootSignatureSubobject(_In_ const ComPtr<ID3D12RootSignature>& pGlobalRootSignature);
        void createPipelineConfigSubobject();

    private:
        ComPtr<ID3D12StateObject> m_stateObject;
        CD3DX12_STATE_OBJECT_DESC m_stateObjectDesc;

    };
}