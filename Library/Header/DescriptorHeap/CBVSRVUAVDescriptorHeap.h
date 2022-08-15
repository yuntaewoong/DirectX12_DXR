#pragma once
#include "Common\Common.h"
#include "DescriptorHeap\DescriptorHeap.h"

namespace library
{
    class CBVSRVUAVDescriptorHeap final : public DescriptorHeap
    {
    public:
        CBVSRVUAVDescriptorHeap();
        CBVSRVUAVDescriptorHeap(const CBVSRVUAVDescriptorHeap& other) = delete;
        CBVSRVUAVDescriptorHeap(CBVSRVUAVDescriptorHeap&& other) = delete;
        CBVSRVUAVDescriptorHeap& operator=(const CBVSRVUAVDescriptorHeap& other) = delete;
        CBVSRVUAVDescriptorHeap& operator=(CBVSRVUAVDescriptorHeap&& other) = delete;
        virtual ~CBVSRVUAVDescriptorHeap() = default;
        HRESULT CreateCBV(
            _In_ const ComPtr<ID3D12Device>& pDevice,
            _In_ const D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc,
            _Out_ D3D12_GPU_DESCRIPTOR_HANDLE* gpuDescriptorHandle
        );
        HRESULT CreateSRV(
            _In_ const ComPtr<ID3D12Device>& pDevice,
            _In_ const ComPtr<ID3D12Resource>& pSRVResource,
            _In_ const D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc,
            _Out_ D3D12_GPU_DESCRIPTOR_HANDLE* gpuDescriptorHandle
        );
        HRESULT CreateUAV(
            _In_ const ComPtr<ID3D12Device>& pDevice,
            _In_ const ComPtr<ID3D12Resource>& pUAVResource,
            _In_ const D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc,
            _Out_ D3D12_GPU_DESCRIPTOR_HANDLE* gpuDescriptorHandle
        );
    private:
        virtual D3D12_DESCRIPTOR_HEAP_DESC createDescriptorHeapDesc() override;
        virtual D3D12_DESCRIPTOR_HEAP_TYPE getDescriptorHeapType() override;
    };
}