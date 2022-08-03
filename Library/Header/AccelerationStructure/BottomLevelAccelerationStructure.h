#pragma once
#include "Common/Common.h"
#include "Render\Renderable.h"
namespace library
{
	class BottomLevelAccelerationStructure
	{
	public:
		BottomLevelAccelerationStructure(_In_ std::shared_ptr<Renderable>& pRenderable);
		BottomLevelAccelerationStructure(const BottomLevelAccelerationStructure& other) = delete;
		BottomLevelAccelerationStructure(BottomLevelAccelerationStructure&& other) = delete;
		BottomLevelAccelerationStructure& operator=(const BottomLevelAccelerationStructure& other) = delete;
		BottomLevelAccelerationStructure& operator=(BottomLevelAccelerationStructure&& other) = delete;
		~BottomLevelAccelerationStructure() = default;

		HRESULT Initialize(
			_In_ ID3D12Device5* pDevice,
			_In_ ID3D12GraphicsCommandList4* pCommandList
		);
		void Update(_In_ FLOAT deltaTime);
		const std::shared_ptr<Renderable>& GetRenderable() const;
		D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const;
	private:
		ComPtr<ID3D12Resource> m_scratchResource;
		ComPtr<ID3D12Resource> m_bottomLevelAccelerationStructure;
		std::shared_ptr<Renderable> m_pRenderable;
	};
}