
#pragma once

#include "Common\Common.h"
#include "Material/Texture.h"
#include "DescriptorHeap\CBVSRVUAVDescriptorHeap.h"

namespace library
{
	class Material
	{
	public:
		Material(_In_ XMFLOAT4 baseColor  = XMFLOAT4(1.0f,20.f/255.f,147.f/255.f,1.0f));
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
		void SetAlbedoTexture(_In_ const std::shared_ptr<Texture>& diffuse);
		void SetNormalTexture(_In_ const std::shared_ptr<Texture>& normal);
		void SetSpecularTexture(_In_ const std::shared_ptr<Texture>& specular);
		void SetRoughnessTexture(_In_ const std::shared_ptr<Texture>& roughness);
		void SetMetallicTexture(_In_ const std::shared_ptr<Texture>& metallic);
		void SetRoughness(_In_ FLOAT roughness);
		void SetMetallic(_In_ FLOAT metallic);
		const std::shared_ptr<Texture> GetAlbedoTexture() const;
		const std::shared_ptr<Texture> GetNormalTexture() const;
		const std::shared_ptr<Texture> GetSpecularTexture() const;
		const std::shared_ptr<Texture> GetRoughnessTexture() const;
		const std::shared_ptr<Texture> GetMetallicTexture() const;
		XMFLOAT4 GetAlbedo() const;
		FLOAT GetRoughness() const;
		FLOAT GetMetallic() const;
		BOOL HasAlbedoTexture() const;
		BOOL HasNormalTexture() const;
		BOOL HasSpecularTexture() const;
		BOOL HasRoughnessTexture() const;
		BOOL HasMetallicTexture() const;
	private:
		std::shared_ptr<Texture> m_albedoTexture;
		std::shared_ptr<Texture> m_normalTexture;
		std::shared_ptr<Texture> m_specularTexture;
		std::shared_ptr<Texture> m_roughnessTexture;
		std::shared_ptr<Texture> m_metallicTexture;
		XMFLOAT4 m_albedo;//기본색상
		FLOAT m_roughness;//거친정도
		FLOAT m_metallic;//금속인 정도
	};
}