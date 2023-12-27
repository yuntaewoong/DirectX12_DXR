#include "pch.h"
#include "Random\RandomGenerator.h"

namespace library
{
	RandomGenerator::RandomGenerator() :
		m_randomConstantBuffer(nullptr)
	{}
	ComPtr<ID3D12Resource>& RandomGenerator::GetConstantBuffer()
	{
		return m_randomConstantBuffer;
	}
	HRESULT RandomGenerator::Initialize(ID3D12Device* pDevice)
	{
		return E_NOTIMPL;
	}
	void RandomGenerator::Update(FLOAT deltaTime)
	{
	}
}