#include "pch.h"
#include "Texture\Material.h"

namespace library
{
	Material::Material() :
		m_diffuseTexture(nullptr),
		m_normalTexture(nullptr),
		m_specularTexture(nullptr),
		m_reflectivity(0.f)
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
		if (m_normalTexture)
		{
			hr = m_normalTexture->Initialize(pDevice, pCommandQueue, cbvSrvUavDescriptorHeap);
			if (FAILED(hr))
			{
				return hr;
			}
		}
		if (m_specularTexture)
		{
			hr = m_specularTexture->Initialize(pDevice, pCommandQueue, cbvSrvUavDescriptorHeap);
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
	void Material::SetNormalTexture(_In_ const std::shared_ptr<Texture>& normal)
	{
		m_normalTexture = normal;
	}
	void Material::SetSpecularTexture(_In_ const std::shared_ptr<Texture>& specular)
	{
		m_specularTexture = specular;
	}
	void Material::SetReflectivity(_In_ FLOAT reflectivity)
	{
		m_reflectivity = reflectivity;
	}
	const std::shared_ptr<Texture> Material::GetDiffuseTexture() const
	{
		return m_diffuseTexture;
	}
	const std::shared_ptr<Texture> Material::GetNormalTexture() const
	{
		return m_normalTexture;
	}
	const std::shared_ptr<Texture> Material::GetSpecularTexture() const
	{
		return m_specularTexture;
	}
	FLOAT Material::GetReflectivity() const
	{
		return m_reflectivity;
	}
	BOOL Material::HasDiffuseTexture() const
	{
		return m_diffuseTexture ? TRUE : FALSE;
	}
	BOOL Material::HasNormalTexture() const
	{
		return m_normalTexture ? TRUE : FALSE;
	}
	BOOL Material::HasSpecularTexture() const
	{
		return m_specularTexture ? TRUE : FALSE;
	}
}