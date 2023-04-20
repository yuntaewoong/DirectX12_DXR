#include "pch.h"
#include "Model/ModelMesh.h"
namespace library
{
	ModelMesh::ModelMesh() :
		Mesh::Mesh(
			XMVectorSet(0.0f, 0.0f, 0.f, 1.0f),
			XMVectorSet(0.f, 0.f, 0.f, 1.0f),
			XMVectorSet(1.f, 1.f, 1.f, 1.f),
			XMFLOAT4()
		)
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