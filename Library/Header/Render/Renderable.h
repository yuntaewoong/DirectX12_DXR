#pragma once

#include "Common/Common.h"
namespace library
{
	class Renderable
	{
	public:
		Renderable(
			_In_ XMVECTOR location,
			_In_ XMVECTOR rotation,
			_In_ XMVECTOR scale
		);
		Renderable(const Renderable& other) = delete;
		Renderable(Renderable&& other) = delete;
		Renderable& operator=(const Renderable& other) = delete;
		Renderable& operator=(Renderable&& other) = delete;
		~Renderable() = default;

		virtual HRESULT Initialize(_In_ ID3D12Device* pDevice) = 0;
		virtual void Update(_In_ FLOAT deltaTime) = 0;
		ComPtr<ID3D12Resource>& GetVertexBuffer();
		ComPtr<ID3D12Resource>& GetIndexBuffer();
		virtual UINT GetNumVertices() const = 0;
		virtual UINT GetNumIndices() const = 0;
		XMMATRIX GetWorldMatrix() const;
	protected:
		const virtual Vertex* GetVertices() const = 0;
		const virtual Index* GetIndices() const = 0;
		HRESULT initialize(_In_ ID3D12Device* pDevice);
	private:
		
		HRESULT createVertexBuffer(_In_ ID3D12Device* pDevice);
		HRESULT createIndexBuffer(_In_ ID3D12Device* pDevice);
	private:
		ComPtr<ID3D12Resource> m_vertexBuffer;
		ComPtr<ID3D12Resource> m_indexBuffer;
		XMMATRIX m_world;
	};
}