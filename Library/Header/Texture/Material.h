
#pragma once

#include "Common\Common.h"
#include "Texture/Texture.h"
#include "DescriptorHeap\CBVSRVUAVDescriptorHeap.h"

namespace library
{
	class Material
	{
	public:
		Material();
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
		void SetReflectivity(_In_ FLOAT reflectivity);
		const std::shared_ptr<Texture> GetDiffuseTexture() const;
		FLOAT GetReflectivity() const;
		BOOL HasDiffuseTexture() const;
	private:
		std::shared_ptr<Texture> m_diffuseTexture;
		FLOAT m_reflectivity;//반사되는 정도 (1:거울, 0:완전 빛 흡수)
	};
}