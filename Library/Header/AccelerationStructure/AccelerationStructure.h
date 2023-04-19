#pragma once

#include "Common/Common.h"
namespace library
{
	/*
		AccelerationStructure Base≈¨∑°Ω∫
	*/
	class AccelerationStructure
	{
	public:
		AccelerationStructure();
		AccelerationStructure(const AccelerationStructure& other) = delete;
		AccelerationStructure(AccelerationStructure&& other) = delete;
		AccelerationStructure& operator=(const AccelerationStructure& other) = delete;
		AccelerationStructure& operator=(AccelerationStructure&& other) = delete;
		~AccelerationStructure() = default;

		ComPtr<ID3D12Resource>& GetAccelerationStructure();
		virtual void Update(_In_ FLOAT deltaTime) = 0;
	protected:
		HRESULT initialize(
			_In_ ID3D12Device5* pDevice,
			_In_ ID3D12GraphicsCommandList4* pCommandList
		);
	private:
		virtual D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS createInput() = 0;
		HRESULT createAccelerationStructureBuffer(
			_In_ ID3D12Device* pDevice,
			_In_ const D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO& preBuildInfo
		);
		HRESULT createScratchBuffer(
			_In_ ID3D12Device* pDevice,
			_In_ const D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO& preBuildInfo
		);
	private:
		ComPtr<ID3D12Resource> m_scratchResource;
		ComPtr<ID3D12Resource> m_accelerationStructureResource;
	};
}