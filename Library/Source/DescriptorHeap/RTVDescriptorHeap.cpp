#include "DescriptorHeap\RTVDescriptorHeap.h"
namespace library
{
	RTVDescriptorHeap::RTVDescriptorHeap(_In_ UINT numDescriptors) :
		DescriptorHeap(numDescriptors)
	{}
	HRESULT RTVDescriptorHeap::CreateRTV(_In_ const ComPtr<ID3D12Device>& pDevice,_In_ const ComPtr<ID3D12Resource>& pRenderTarget)
	{
		HRESULT hr = S_OK;
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvCPUHandle{};
		hr = getEmptyDescriptorSpaceCPUHandle(&rtvCPUHandle);
		if (FAILED(hr))
		{
			return hr;
		}
		pDevice->CreateRenderTargetView(pRenderTarget.Get(), nullptr, rtvCPUHandle);
		m_numAllocated++;
	}
	D3D12_DESCRIPTOR_HEAP_DESC RTVDescriptorHeap::createDescriptorHeapDesc()
	{
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {
			.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
			.NumDescriptors = m_numDescriptor,
			.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
			.NodeMask = 0u
		};
		return rtvHeapDesc;
	}
	D3D12_DESCRIPTOR_HEAP_TYPE RTVDescriptorHeap::getDescriptorHeapType()
	{
		return D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	}
}