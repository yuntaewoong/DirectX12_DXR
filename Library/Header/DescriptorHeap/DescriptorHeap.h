#pragma once
#include "Common\Common.h"

namespace library
{
    class DescriptorHeap
    {
    public:
        DescriptorHeap(_In_ UINT numDescriptors);
        DescriptorHeap(const DescriptorHeap& other) = delete;
        DescriptorHeap(DescriptorHeap&& other) = delete;
        DescriptorHeap& operator=(const DescriptorHeap& other) = delete;
        DescriptorHeap& operator=(DescriptorHeap&& other) = delete;
        virtual ~DescriptorHeap() = default;
        HRESULT Initialize(_In_ const ComPtr<ID3D12Device>& pDevice);
        ComPtr<ID3D12DescriptorHeap>& GetDescriptorHeap();
    protected:
        HRESULT getEmptyDescriptorSpaceCPUHandle(_Out_ CD3DX12_CPU_DESCRIPTOR_HANDLE* pCPUHandle);
        HRESULT getLastAllocatedDescriptorGPUHandle(_Out_ CD3DX12_GPU_DESCRIPTOR_HANDLE* pGPUHandle);
    protected:
        ComPtr<ID3D12DescriptorHeap> m_descriptorHeap;
        UINT m_numDescriptor;
        UINT m_descriptorSize;
        UINT m_numAllocated;
    private:
        virtual D3D12_DESCRIPTOR_HEAP_DESC createDescriptorHeapDesc() = 0;
        virtual D3D12_DESCRIPTOR_HEAP_TYPE getDescriptorHeapType() = 0;
    
    };
}