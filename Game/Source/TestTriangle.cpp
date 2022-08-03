#include "TestTriangle.h"


TestTriangle::TestTriangle()
{}
HRESULT TestTriangle::Initialize(_In_ ID3D12Device* pDevice)
{
	HRESULT hr = S_OK;
	hr = Renderable::initialize(pDevice);
	if (FAILED(hr))
	{
		return hr;
	}
}
void TestTriangle::Update(_In_ FLOAT deltaTime)
{}
UINT TestTriangle::GetNumVertices() const
{
	return ARRAYSIZE(VERTICES);
}
UINT TestTriangle::GetNumIndices() const
{
	return ARRAYSIZE(INDICES);
}
const Vertex* TestTriangle::GetVertices() const
{
	return VERTICES;
}
const Index* TestTriangle::GetIndices() const
{
	return INDICES;
}