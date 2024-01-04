#include "pch.h"
#include "DescriptorHeap\DescriptorHeap.h"

namespace library
{
	DescriptorHeap::DescriptorHeap(_In_ UINT numDescriptors) :
		m_descriptorHeap(nullptr),
		m_numDescriptor(numDescriptors),
		m_descriptorSize(0u),
		m_numAllocated(0u)
	{}
	HRESULT DescriptorHeap::Initialize(_In_ const ComPtr<ID3D12Device>& pDevice)
	{
		HRESULT hr = S_OK;
		D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = createDescriptorHeapDesc();
		hr = pDevice->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&m_descriptorHeap));//create
		if (FAILED(hr))
		{
			return hr;
		}
		m_descriptorSize = pDevice->GetDescriptorHandleIncrementSize(getDescriptorHeapType());//gpu별로 상이한 descriptor사이즈 가져오기
		return hr;
	}
	ComPtr<ID3D12DescriptorHeap>& DescriptorHeap::GetDescriptorHeap()
	{
		return m_descriptorHeap;
	}
	HRESULT DescriptorHeap::getEmptyDescriptorSpaceCPUHandle(_Out_ CD3DX12_CPU_DESCRIPTOR_HANDLE* pCPUHandle)
	{
		if (m_numAllocated >= m_numDescriptor)
		{
			return E_FAIL;//메모리 부족
		}
		pCPUHandle->InitOffsetted(
			m_descriptorHeap->GetCPUDescriptorHandleForHeapStart(),
			m_numAllocated,
			m_descriptorSize
		);
		return S_OK;
	}
	HRESULT DescriptorHeap::getLastAllocatedDescriptorGPUHandle(_Out_ CD3DX12_GPU_DESCRIPTOR_HANDLE* pGPUHandle)
	{
		pGPUHandle->InitOffsetted(
			m_descriptorHeap->GetGPUDescriptorHandleForHeapStart(),
			m_numAllocated,
			m_descriptorSize
		);
		return S_OK;
	}
}