#include "pch.h"
#include "DescriptorHeap\RTVDescriptorHeap.h"
namespace library
{
	RTVDescriptorHeap::RTVDescriptorHeap(_In_ UINT numDescriptors) :
		DescriptorHeap(numDescriptors),
		m_RTVCPUHandles(std::vector<D3D12_CPU_DESCRIPTOR_HANDLE>())
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
		m_RTVCPUHandles.push_back(static_cast<D3D12_CPU_DESCRIPTOR_HANDLE>(rtvCPUHandle.ptr));
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

		pDevice->CreateRenderTargetView(pRenderTarget.Get(), &rtvDesc, rtvCPUHandle);
		m_numAllocated++;
		return hr;
	}
	D3D12_CPU_DESCRIPTOR_HANDLE RTVDescriptorHeap::GetRTVCPUHandle(UINT index) const
	{
		return m_RTVCPUHandles[index];
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