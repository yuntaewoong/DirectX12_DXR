#pragma once

#include "Common/Common.h"
#include "AccelerationStructure\TopLevelAccelerationStructure.h"
#include "AccelerationStructure\BottomLevelAccelerationStructure.h"
#include "Render\Renderable.h"
namespace library
{
	class AccelerationStructure
	{
	public:
		AccelerationStructure();
		AccelerationStructure(const AccelerationStructure& other) = delete;
		AccelerationStructure(AccelerationStructure&& other) = delete;
		AccelerationStructure& operator=(const AccelerationStructure& other) = delete;
		AccelerationStructure& operator=(AccelerationStructure&& other) = delete;
		~AccelerationStructure() = default;

		HRESULT Initialize(
			_In_ ID3D12Device5* pDevice,
			_In_ ID3D12GraphicsCommandList4* pCommandList
		);
		void Update(_In_ FLOAT deltaTime);
		void AddRenderable(_In_ std::shared_ptr<Renderable>& pRenderable);//1개의 Renderable당 1개의 BLAS생성
		const std::unique_ptr<TopLevelAccelerationStructure>& GetTLAS() const;
		const std::vector<std::unique_ptr<BottomLevelAccelerationStructure>>& GetBLASVector() const;
	private:
		std::unique_ptr<TopLevelAccelerationStructure> m_topLevelAccelerationStructure;
		std::vector<std::unique_ptr<BottomLevelAccelerationStructure>> m_bottomLevelAccelerationStructures;
	};
}