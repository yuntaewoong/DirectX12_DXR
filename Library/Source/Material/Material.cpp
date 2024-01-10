#include "pch.h"
#include "Material\Material.h"

namespace library
{
	Material::Material(_In_ XMFLOAT4 baseColor) :
		m_albedoTexture(nullptr),
		m_normalTexture(nullptr),
		m_specularTexture(nullptr),
		m_roughnessTexture(nullptr),
		m_metallicTexture(nullptr),
		m_albedo(baseColor),
		m_roughness(0.0f),
		m_metallic(0.0f),
		m_emission(0.0f)
	{}

	HRESULT Material::Initialize(
		_In_ const ComPtr<ID3D12Device>& pDevice,
		_In_ const ComPtr<ID3D12CommandQueue>& pCommandQueue,
		_In_ CBVSRVUAVDescriptorHeap& cbvSrvUavDescriptorHeap
	)
	{
		HRESULT hr = S_OK;
		if (m_albedoTexture)
		{
			hr = m_albedoTexture->Initialize(pDevice, pCommandQueue, cbvSrvUavDescriptorHeap);
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
		if (m_roughnessTexture)
		{
			hr = m_roughnessTexture->Initialize(pDevice, pCommandQueue, cbvSrvUavDescriptorHeap);
			if (FAILED(hr))
			{
				return hr;
			}
		}
		if (m_metallicTexture)
		{
			hr = m_metallicTexture->Initialize(pDevice, pCommandQueue, cbvSrvUavDescriptorHeap);
			if (FAILED(hr))
			{
				return hr;
			}
		}
		return hr;
	}

	void Material::SetAlbedoTexture(_In_ const std::shared_ptr<Texture>& diffuse)
	{
		m_albedoTexture = diffuse;
	}
	void Material::SetNormalTexture(_In_ const std::shared_ptr<Texture>& normal)
	{
		m_normalTexture = normal;
	}
	void Material::SetSpecularTexture(_In_ const std::shared_ptr<Texture>& specular)
	{
		m_specularTexture = specular;
	}
	void Material::SetRoughnessTexture(_In_ const std::shared_ptr<Texture>& roughness)
	{
		m_roughnessTexture = roughness;
	}
	void Material::SetMetallicTexture(_In_ const std::shared_ptr<Texture>& metallic)
	{
		m_metallicTexture = metallic;
	}
	void Material::SetRoughness(_In_ FLOAT roughness)
	{
		m_roughness = roughness;
	}
	void Material::SetMetallic(_In_ FLOAT metallic)
	{
		m_metallic = metallic;
	}
	void Material::SetEmission(_In_ FLOAT emission)
	{
		m_emission = emission;
	}
	const std::shared_ptr<Texture> Material::GetAlbedoTexture() const
	{
		return m_albedoTexture;
	}
	const std::shared_ptr<Texture> Material::GetNormalTexture() const
	{
		return m_normalTexture;
	}
	const std::shared_ptr<Texture> Material::GetSpecularTexture() const
	{
		return m_specularTexture;
	}
	const std::shared_ptr<Texture> Material::GetRoughnessTexture() const
	{
		return m_roughnessTexture;
	}
	const std::shared_ptr<Texture> Material::GetMetallicTexture() const
	{
		return m_metallicTexture;
	}
	XMFLOAT4 Material::GetAlbedo() const
	{
		return m_albedo;
	}
	FLOAT Material::GetRoughness() const
	{
		return m_roughness;
	}
	FLOAT Material::GetMetallic() const
	{
		return m_metallic;
	}
	FLOAT Material::GetEmission() const
	{
		return m_emission;
	}
	BOOL Material::HasAlbedoTexture() const
	{
		return m_albedoTexture ? TRUE : FALSE;
	}
	BOOL Material::HasNormalTexture() const
	{
		return m_normalTexture ? TRUE : FALSE;
	}
	BOOL Material::HasSpecularTexture() const
	{
		return m_specularTexture ? TRUE : FALSE;
	}
	BOOL Material::HasRoughnessTexture() const
	{
		return m_roughnessTexture ? TRUE : FALSE;
	}
	BOOL Material::HasMetallicTexture() const
	{
		return m_metallicTexture ? TRUE : FALSE;
	}
}