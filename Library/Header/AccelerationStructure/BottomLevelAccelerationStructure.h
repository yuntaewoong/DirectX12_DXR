#pragma once
#include "Common/Common.h"
#include "Render\Mesh.h"
#include "AccelerationStructure\AccelerationStructure.h"
namespace library
{
	class BottomLevelAccelerationStructure final : public AccelerationStructure
	{
	public:
		BottomLevelAccelerationStructure(_In_ const std::shared_ptr<Mesh>& pMesh);
		BottomLevelAccelerationStructure(const BottomLevelAccelerationStructure& other) = delete;
		BottomLevelAccelerationStructure(BottomLevelAccelerationStructure&& other) = delete;
		BottomLevelAccelerationStructure& operator=(const BottomLevelAccelerationStructure& other) = delete;
		BottomLevelAccelerationStructure& operator=(BottomLevelAccelerationStructure&& other) = delete;
		virtual ~BottomLevelAccelerationStructure() = default;

		HRESULT Initialize(
			_In_ ID3D12Device5* pDevice,
			_In_ ID3D12GraphicsCommandList4* pCommandList
		);
		virtual void Update(_In_ FLOAT deltaTime) override;
		const std::shared_ptr<Mesh>& GetMesh() const;
	private:
		virtual D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS createInput()  override;
		void createGeometryDesc();
	private:
		std::shared_ptr<Mesh> m_pMesh;
		D3D12_RAYTRACING_GEOMETRY_DESC m_geometryDesc;
	}; 

}