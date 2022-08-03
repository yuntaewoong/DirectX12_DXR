#include "AccelerationStructure\AccelerationStructure.h"

namespace library
{
	AccelerationStructure::AccelerationStructure() :
		m_topLevelAccelerationStructure(std::make_unique<TopLevelAccelerationStructure>()),
		m_bottomLevelAccelerationStructures(std::vector<std::unique_ptr<BottomLevelAccelerationStructure>>())
	{}
	HRESULT AccelerationStructure::Initialize(
		_In_ ID3D12Device5* pDevice,
		_In_ ID3D12GraphicsCommandList4* pCommandList
	)
	{
		HRESULT hr = S_OK;
		for (auto& iBlas : m_bottomLevelAccelerationStructures)
		{
			hr = iBlas->Initialize(pDevice, pCommandList);
			if (FAILED(hr))
			{
				return hr;
			}
		}
		hr = m_topLevelAccelerationStructure->Initialize(pDevice, pCommandList,m_bottomLevelAccelerationStructures);
		if (FAILED(hr))
		{
			return hr;
		}
		return hr;
	}
	void AccelerationStructure::Update(_In_ FLOAT deltaTime)
	{
		m_topLevelAccelerationStructure->Update(deltaTime);
		for (auto& iBlas : m_bottomLevelAccelerationStructures)
		{
			iBlas->Update(deltaTime);
		}
	}
	void AccelerationStructure::AddRenderable(_In_ std::shared_ptr<Renderable>& pRenderable)
	{
		m_bottomLevelAccelerationStructures.push_back(std::make_unique<BottomLevelAccelerationStructure>(pRenderable));
	}
	const std::unique_ptr<TopLevelAccelerationStructure>& AccelerationStructure::GetTLAS() const
	{
		return m_topLevelAccelerationStructure;
	}
	const std::vector<std::unique_ptr<BottomLevelAccelerationStructure>>& AccelerationStructure::GetBLASVector() const
	{
		return m_bottomLevelAccelerationStructures;
	}
}