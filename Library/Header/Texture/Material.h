
#pragma once

#include "Common\Common.h"
#include "Texture/Texture.h"
#include "DescriptorHeap\CBVSRVUAVDescriptorHeap.h"

namespace library
{
	class Material
	{
	public:
		Material(_In_ MaterialType::Enum materialType);
		Material(const Material& other) = default;
		Material(Material&& other) = default;
		Material& operator=(const Material& other) = default;
		Material& operator=(Material&& other) = default;
		virtual ~Material() = default;

		HRESULT Initialize(
			_In_ const ComPtr<ID3D12Device>& pDevice,
			_In_ const ComPtr<ID3D12CommandQueue>& pCommandQueue,
			_In_ CBVSRVUAVDescriptorHeap& cbvSrvUavDescriptorHeap
		);
		void SetDiffuseTexture(_In_ const std::shared_ptr<Texture>& diffuse);
		void SetNormalTexture(_In_ const std::shared_ptr<Texture>& normal);
		void SetSpecularTexture(_In_ const std::shared_ptr<Texture>& specular);
		void SetRoughnessTexture(_In_ const std::shared_ptr<Texture>& roughness);
		void SetMetallicTexture(_In_ const std::shared_ptr<Texture>& metallic);
		void SetReflectivity(_In_ FLOAT reflectivity);
		void SetRoughness(_In_ FLOAT roughness);
		void SetMetallic(_In_ FLOAT metallic);
		const std::shared_ptr<Texture> GetDiffuseTexture() const;
		const std::shared_ptr<Texture> GetNormalTexture() const;
		const std::shared_ptr<Texture> GetSpecularTexture() const;
		const std::shared_ptr<Texture> GetRoughnessTexture() const;
		const std::shared_ptr<Texture> GetMetallicTexture() const;
		MaterialType::Enum GetMaterialType() const;
		FLOAT GetReflectivity() const;
		FLOAT GetRoughness() const;
		FLOAT GetMetallic() const;
		BOOL HasDiffuseTexture() const;
		BOOL HasNormalTexture() const;
		BOOL HasSpecularTexture() const;
		BOOL HasRoughnessTexture() const;
		BOOL HasMetallicTexture() const;
	private:
		MaterialType::Enum m_materialType;
		std::shared_ptr<Texture> m_diffuseTexture;
		std::shared_ptr<Texture> m_normalTexture;
		std::shared_ptr<Texture> m_specularTexture;
		std::shared_ptr<Texture> m_roughnessTexture;
		std::shared_ptr<Texture> m_metallicTexture;
		FLOAT m_reflectivity;//반사되는 정도 (1:거울, 0:완전 빛 흡수)
		FLOAT m_roughness;//거친정도
		FLOAT m_metallic;//금속인 정도
	};
}