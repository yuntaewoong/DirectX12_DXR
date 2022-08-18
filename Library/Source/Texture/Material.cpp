#include "pch.h"
#include "Texture\Material.h"

namespace library
{
	Material::Material() :
		m_diffuseTexture(nullptr)
	{}

	HRESULT Material::Initialize(
		_In_ const ComPtr<ID3D12Device>& pDevice,
		_In_ const ComPtr<ID3D12CommandQueue>& pCommandQueue,
		_In_ CBVSRVUAVDescriptorHeap& cbvSrvUavDescriptorHeap
	)
	{
		HRESULT hr = S_OK;
		if (m_diffuseTexture)
		{
			hr = m_diffuseTexture->Initialize(pDevice, pCommandQueue, cbvSrvUavDescriptorHeap);
			if (FAILED(hr))
			{
				return hr;
			}
		}
		return hr;
	}

	void Material::SetDiffuseTexture(_In_ const std::shared_ptr<Texture>& diffuse)
	{
		m_diffuseTexture = diffuse;
	}
	const std::shared_ptr<Texture> Material::GetDiffuseTexture() const
	{
		return m_diffuseTexture;
	}
	BOOL Material::HasDiffuseTexture() const
	{
		if (m_diffuseTexture)
			return TRUE;
		else
			return FALSE;
	}
}