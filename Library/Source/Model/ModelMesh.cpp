#include "pch.h"
#include "Model/ModelMesh.h"
namespace library
{
	ModelMesh::ModelMesh(
		_In_ XMVECTOR location,
		_In_ XMVECTOR rotation,
		_In_ XMVECTOR scale,
		_In_ XMFLOAT4 color
	) 
		:
		Mesh::Mesh(
			location,
			rotation,
			scale,
			color
		),
		m_vertices(std::vector<Vertex>()),
		m_indices(std::vector<Index>())
	{}
	void ModelMesh::AddVertex(Vertex vertex)
	{
		m_vertices.push_back(vertex);
	}
	void ModelMesh::AddIndex(Index index)
	{
		m_indices.push_back(index);
	}
	HRESULT ModelMesh::Initialize(_In_ const ComPtr<ID3D12Device>& pDevice)
	{
		HRESULT hr = S_OK;
		hr = Mesh::initialize(pDevice);
		if (FAILED(hr))
		{
			return hr;
		}
		return hr;
	}
	void ModelMesh::Update(_In_ FLOAT deltaTime)
	{

	}
	UINT ModelMesh::GetNumVertices() const
	{
		return m_vertices.size();
	}
	UINT ModelMesh::GetNumIndices() const
	{
		return m_indices.size();
	}
	const Vertex* ModelMesh::GetVertices() const
	{
		return m_vertices.data();
	}
	const Index* ModelMesh::GetIndices() const
	{
		return m_indices.data();
	}
}