#include "pch.h"
#include "DescriptorHeap\CBVSRVUAVDescriptorHeap.h"
namespace library
{
	CBVSRVUAVDescriptorHeap::CBVSRVUAVDescriptorHeap() :
		DescriptorHeap::DescriptorHeap(NUM_DESCRIPTORS_DEFAULT)
	{}
	HRESULT CBVSRVUAVDescriptorHeap::CreateCBV(
		_In_ const ComPtr<ID3D12Device>& pDevice,
		_In_ const D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc,
		_Out_ D3D12_GPU_DESCRIPTOR_HANDLE* gpuDescriptorHandle
	)
	{
		HRESULT hr = S_OK;
		CD3DX12_CPU_DESCRIPTOR_HANDLE cbvCPUHandle{};
		hr = getEmptyDescriptorSpaceCPUHandle(&cbvCPUHandle);
		if (FAILED(hr))
		{
			return hr;
		}
		pDevice->CreateConstantBufferView(&cbvDesc, cbvCPUHandle);
		CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle{};
		getLastAllocatedDescriptorGPUHandle(&gpuHandle);
		*gpuDescriptorHandle = gpuHandle;
		m_numAllocated++;
		return hr;
	}
	HRESULT CBVSRVUAVDescriptorHeap::CreateSRV(
		_In_ const ComPtr<ID3D12Device>& pDevice, 
		_In_ const ComPtr<ID3D12Resource>& pSRVResource,
		_In_ const D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc,
		_Out_ D3D12_GPU_DESCRIPTOR_HANDLE* gpuDescriptorHandle
	)
	{
		HRESULT hr = S_OK;
		CD3DX12_CPU_DESCRIPTOR_HANDLE srvCPUHandle{};
		hr = getEmptyDescriptorSpaceCPUHandle(&srvCPUHandle);
		if (FAILED(hr))
		{
			return hr;
		}
		pDevice->CreateShaderResourceView(pSRVResource.Get(), &srvDesc, srvCPUHandle);
		CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle{};
		getLastAllocatedDescriptorGPUHandle(&gpuHandle);
		*gpuDescriptorHandle = gpuHandle;
		m_numAllocated++;
		return hr;
	}
	HRESULT CBVSRVUAVDescriptorHeap::CreateUAV(
		_In_ const ComPtr<ID3D12Device>& pDevice,
		_In_ const ComPtr<ID3D12Resource>& pUAVResource,
		_In_ const D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc,
		_Out_ D3D12_GPU_DESCRIPTOR_HANDLE* gpuDescriptorHandle
	)
	{
		HRESULT hr = S_OK;
		CD3DX12_CPU_DESCRIPTOR_HANDLE uavCPUHandle{};
		hr = getEmptyDescriptorSpaceCPUHandle(&uavCPUHandle);
		if (FAILED(hr))
		{
			return hr;
		}
		pDevice->CreateUnorderedAccessView(pUAVResource.Get(), nullptr, &uavDesc, uavCPUHandle);
		CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle{};
		getLastAllocatedDescriptorGPUHandle(&gpuHandle);
		*gpuDescriptorHandle = gpuHandle;
		m_numAllocated++;
		return hr;
	}
	D3D12_DESCRIPTOR_HEAP_DESC CBVSRVUAVDescriptorHeap::createDescriptorHeapDesc()
	{
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {
			.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
			.NumDescriptors = m_numDescriptor,
			.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, //CBV,SRV,UAV는 Shader에서 참조해야함
			.NodeMask = 0u
		};
		return heapDesc;
	}
	D3D12_DESCRIPTOR_HEAP_TYPE CBVSRVUAVDescriptorHeap::getDescriptorHeapType()
	{
		return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	}
}