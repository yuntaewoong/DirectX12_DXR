//모델을 구성하는 Mesh 클래스(런타임에  Assimp로딩을 해서 일반적인 Mesh랑 구분해서 구현)
#pragma once
#include "Render\Mesh.h"
namespace library
{
	class ModelMesh : public Mesh
	{
	public:
		ModelMesh(
			_In_ XMVECTOR location,
			_In_ XMVECTOR rotation,
			_In_ XMVECTOR scale,
			_In_ XMFLOAT4 color
		);
		ModelMesh(const ModelMesh& other) = delete;
		ModelMesh(ModelMesh&& other) = delete;
		ModelMesh& operator=(const ModelMesh& other) = delete;
		ModelMesh& operator=(ModelMesh&& other) = delete;
		~ModelMesh() = default;

		void AddVertex(Vertex vertex);
		void AddIndex(Index index);

		virtual HRESULT Initialize(_In_ const ComPtr<ID3D12Device>& pDevice) override;
		void Update(_In_ FLOAT deltaTime) override;
		virtual UINT GetNumVertices() const override;
		virtual UINT GetNumIndices() const override;
	protected:
		const virtual Vertex* GetVertices() const override;
		const virtual Index* GetIndices() const override;
	private:
		std::vector<Vertex> m_vertices;
		std::vector<Index> m_indices;
	};
}