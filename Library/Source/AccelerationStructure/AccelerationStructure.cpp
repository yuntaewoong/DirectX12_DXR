#include "pch.h"
#include "AccelerationStructure\AccelerationStructure.h"

namespace library
{
	AccelerationStructure::AccelerationStructure() :
		m_scratchResource(nullptr),
		m_accelerationStructureResource(nullptr)
	{}
	ComPtr<ID3D12Resource>& AccelerationStructure::GetAccelerationStructure()
	{
		return m_accelerationStructureResource;
	}
	HRESULT AccelerationStructure::initialize(
		_In_ ID3D12Device5* pDevice,
		_In_ ID3D12GraphicsCommandList4* pCommandList
	)
	{
		HRESULT hr = S_OK;
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO preBuildInfo = {};
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = createInput();
		pDevice->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &preBuildInfo);
		hr = createScratchBuffer(pDevice, preBuildInfo);
		if (FAILED(hr))
		{
			return hr;
		}
		hr = createAccelerationStructureBuffer(pDevice, preBuildInfo);
		if (FAILED(hr))
		{
			return hr;
		}
		{
			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC  buildDesc = {
				.DestAccelerationStructureData = m_accelerationStructureResource->GetGPUVirtualAddress(), //버퍼 주소
				.Inputs = inputs,                                                                          //인풋
				.SourceAccelerationStructureData = 0,                                                       //?
				.ScratchAccelerationStructureData = m_scratchResource->GetGPUVirtualAddress()                 //scratch버퍼 주소
			};
			pCommandList->BuildRaytracingAccelerationStructure(&buildDesc, 0, nullptr);
		}
		return S_OK;
	}
	HRESULT AccelerationStructure::createAccelerationStructureBuffer(
		_In_ ID3D12Device* pDevice,
		_In_ const D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO& preBuildInfo
	)
	{
		CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
		CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(
			preBuildInfo.ResultDataMaxSizeInBytes,
			D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
		);
		HRESULT hr = pDevice->CreateCommittedResource(
			&uploadHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&bufferDesc,
			D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,//resource state는 acceleration structure,
			nullptr,
			IID_PPV_ARGS(&m_accelerationStructureResource)
		);
		if (FAILED(hr))
		{
			return hr;
		}
		return hr;
	}
	HRESULT AccelerationStructure::createScratchBuffer(
		_In_ ID3D12Device* pDevice,
		_In_ const D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO& preBuildInfo
	)
	{
		CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
		CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(
			preBuildInfo.ScratchDataSizeInBytes,
			D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
		);
		HRESULT hr = pDevice->CreateCommittedResource(
			&uploadHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&bufferDesc,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(&m_scratchResource)
		);
		if (FAILED(hr))
		{
			return hr;
		}
		return hr;
	}
}