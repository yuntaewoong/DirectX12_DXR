#pragma once

#include "Common\Common.h"
#include "Texture\Material.h"
namespace library
{
	class Mesh
	{
	public:
		Mesh(
			_In_ XMVECTOR location,
			_In_ XMVECTOR rotation,
			_In_ XMVECTOR scale,
			_In_ XMFLOAT4 color
		);
		Mesh(const Mesh& other) = delete;
		Mesh(Mesh&& other) = delete;
		Mesh& operator=(const Mesh& other) = delete;
		Mesh& operator=(Mesh&& other) = delete;
		~Mesh() = default;

		virtual HRESULT Initialize(_In_ const ComPtr<ID3D12Device>& pDevice) = 0;
		virtual void Update(_In_ FLOAT deltaTime) = 0;
		ComPtr<ID3D12Resource>& GetVertexBuffer();
		ComPtr<ID3D12Resource>& GetIndexBuffer();
		virtual UINT GetNumVertices() const = 0;
		virtual UINT GetNumIndices() const = 0;
		XMMATRIX GetWorldMatrix() const;
		XMFLOAT4 GetColor() const;
		const std::shared_ptr<Material>& GetMaterial() const;
		void SetMaterial(_In_ const std::shared_ptr<Material>& pMaterial);
	protected:
		const virtual Vertex* GetVertices() const = 0;
		const virtual Index* GetIndices() const = 0;
		HRESULT initialize(_In_ const ComPtr<ID3D12Device>& pDevice);
	private:
		HRESULT createVertexBuffer(_In_ const ComPtr<ID3D12Device>& pDevice);
		HRESULT createIndexBuffer(_In_ const ComPtr<ID3D12Device>& pDevice);
	private:
		ComPtr<ID3D12Resource> m_vertexBuffer;
		ComPtr<ID3D12Resource> m_indexBuffer;
		XMMATRIX m_world;
		XMFLOAT4 m_color;
		std::shared_ptr<Material> m_material;
	};
}