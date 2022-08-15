#pragma once
#include "DescriptorHeap\DescriptorHeap.h"

namespace library
{
    class RTVDescriptorHeap final : public DescriptorHeap
    {
    public:
        RTVDescriptorHeap(_In_ UINT numDescriptors);
        RTVDescriptorHeap(const RTVDescriptorHeap& other) = delete;
        RTVDescriptorHeap(RTVDescriptorHeap&& other) = delete;
        RTVDescriptorHeap& operator=(const RTVDescriptorHeap& other) = delete;
        RTVDescriptorHeap& operator=(RTVDescriptorHeap&& other) = delete;
        virtual ~RTVDescriptorHeap() = default;
        HRESULT CreateRTV(_In_ const ComPtr<ID3D12Device>& pDevice,_In_ const ComPtr<ID3D12Resource>& pRenderTarget);
    private:
        virtual D3D12_DESCRIPTOR_HEAP_DESC createDescriptorHeapDesc() override;
        virtual D3D12_DESCRIPTOR_HEAP_TYPE getDescriptorHeapType() override;
    };
}