#pragma once

#include "Common/Common.h"
#include "AccelerationStructure\BottomLevelAccelerationStructure.h"
#include "AccelerationStructure\AccelerationStructure.h"
namespace library
{
	class TopLevelAccelerationStructure final : public AccelerationStructure
	{
	public:
		TopLevelAccelerationStructure();
		TopLevelAccelerationStructure(const TopLevelAccelerationStructure& other) = delete;
		TopLevelAccelerationStructure(TopLevelAccelerationStructure&& other) = delete;
		TopLevelAccelerationStructure& operator=(const TopLevelAccelerationStructure& other) = delete;
		TopLevelAccelerationStructure& operator=(TopLevelAccelerationStructure&& other) = delete;
		virtual ~TopLevelAccelerationStructure() = default;

		HRESULT Initialize(
			_In_ ID3D12Device5* pDevice,
			_In_ ID3D12GraphicsCommandList4* pCommandList,
			_In_ const std::vector<std::unique_ptr<BottomLevelAccelerationStructure>>& m_bottomLevelAccelerationStructures
		);
		virtual void Update(_In_ FLOAT deltaTime) override;
	private:
		virtual D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS createInput() override;
		HRESULT createInstanceBuffer(
			_In_ ID3D12Device* pDevice,
			_In_ const std::vector<std::unique_ptr<BottomLevelAccelerationStructure>>& m_bottomLevelAccelerationStructures
		);
	private:
		ComPtr<ID3D12Resource> m_instanceResource;
		std::vector<D3D12_RAYTRACING_INSTANCE_DESC> m_instanceDescs;
	};
}