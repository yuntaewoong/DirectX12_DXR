#pragma once

#include "Common\Common.h"
#include "Material\Material.h"
namespace library
{
	class Mesh
	{
	public:
		Mesh(
			_In_ XMVECTOR location,
			_In_ XMVECTOR rotation,
			_In_ XMVECTOR scale
		);
		Mesh(const Mesh& other) = delete;
		Mesh(Mesh&& other) = delete;
		Mesh& operator=(const Mesh& other) = delete;
		Mesh& operator=(Mesh&& other) = delete;
		~Mesh() = default;

		HRESULT Initialize(_In_ const ComPtr<ID3D12Device>& pDevice);
		virtual void Update(_In_ FLOAT deltaTime);
		ComPtr<ID3D12Resource>& GetVertexBuffer();
		ComPtr<ID3D12Resource>& GetIndexBuffer();
		const std::vector<Vertex>& GetVertices() const;
		const std::vector<Index>& GetIndices() const;
		XMMATRIX GetWorldMatrix() const;
		const std::shared_ptr<Material>& GetMaterial() const;
		void SetMaterial(_In_ const std::shared_ptr<Material>& pMaterial);
		void AddVertex(Vertex vertex);
		void AddIndex(Index index);
	private:
		HRESULT createVertexBuffer(_In_ const ComPtr<ID3D12Device>& pDevice);
		HRESULT createIndexBuffer(_In_ const ComPtr<ID3D12Device>& pDevice);
	private:
		ComPtr<ID3D12Resource> m_vertexBuffer;
		ComPtr<ID3D12Resource> m_indexBuffer;
		XMMATRIX m_world;
		std::shared_ptr<Material> m_material;
		std::vector<Vertex> m_vertices;
		std::vector<Index> m_indices;
	};
}