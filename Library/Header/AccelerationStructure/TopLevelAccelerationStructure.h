#pragma once

#include "Common/Common.h"
#include "AccelerationStructure\BottomLevelAccelerationStructure.h"
namespace library
{
	class TopLevelAccelerationStructure
	{
	public:
		TopLevelAccelerationStructure();
		TopLevelAccelerationStructure(const TopLevelAccelerationStructure& other) = delete;
		TopLevelAccelerationStructure(TopLevelAccelerationStructure&& other) = delete;
		TopLevelAccelerationStructure& operator=(const TopLevelAccelerationStructure& other) = delete;
		TopLevelAccelerationStructure& operator=(TopLevelAccelerationStructure&& other) = delete;
		~TopLevelAccelerationStructure() = default;

		HRESULT Initialize(
			_In_ ID3D12Device5* pDevice,
			_In_ ID3D12GraphicsCommandList4* pCommandList,
			_In_ const std::vector<std::unique_ptr<BottomLevelAccelerationStructure>>& m_bottomLevelAccelerationStructures
		);
		D3D12_GPU_VIRTUAL_ADDRESS GetTLASVirtualAddress() const;
		void Update(_In_ FLOAT deltaTime);
	private:
		ComPtr<ID3D12Resource> m_scratchResource;
		ComPtr<ID3D12Resource> m_topLevelAccelerationStructure;
		ComPtr<ID3D12Resource> m_instanceResource;
	};
}