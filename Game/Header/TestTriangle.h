#pragma once
#include "Common\Common.h"
#include "Render\Renderable.h"

class TestTriangle : public library::Renderable
{
public:
	TestTriangle(
		_In_ XMVECTOR location,
		_In_ XMVECTOR rotation,
		_In_ XMVECTOR scale
	);
	TestTriangle(const Renderable& other) = delete;
	TestTriangle(Renderable&& other) = delete;
	TestTriangle& operator=(const Renderable& other) = delete;
	TestTriangle& operator=(Renderable&& other) = delete;
	~TestTriangle() = default;

	HRESULT Initialize(_In_ ID3D12Device* pDevice) override;
	void Update(_In_ FLOAT deltaTime) override;
	virtual UINT GetNumVertices() const override;
	virtual UINT GetNumIndices() const override;
	const virtual Vertex* GetVertices() const override;
	const virtual Index* GetIndices() const override;
private:
	static constexpr const Vertex VERTICES[] = 
	{
		{XMFLOAT3(0, -1.f, 0.f)},
		{ XMFLOAT3(-1.f,1.f,0.f) },
		{ XMFLOAT3(1.f,1.f,0.f) }
	};
	static constexpr const Index INDICES[] =
	{
		{0},{1},{2}
	};
};