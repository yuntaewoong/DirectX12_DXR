#include "pch_game.h"
#include "Plane\BasePlane.h"

BasePlane::BasePlane(
	_In_ XMVECTOR location,
	_In_ XMVECTOR rotation,
	_In_ XMVECTOR scale,
	_In_ XMFLOAT4 color
) :
	Mesh::Mesh(location, rotation, scale, color)
{}
HRESULT BasePlane::Initialize(_In_ const ComPtr<ID3D12Device>& pDevice)
{
	HRESULT hr = S_OK;
	hr = Mesh::initialize(pDevice);
	if (FAILED(hr))
	{
		return hr;
	}
	return hr;
}
void BasePlane::Update(_In_ FLOAT deltaTime)
{

}
UINT BasePlane::GetNumVertices() const
{
	return ARRAYSIZE(VERTICES);
}
UINT BasePlane::GetNumIndices() const
{
	return ARRAYSIZE(INDICES);
}
const Vertex* BasePlane::GetVertices() const
{
	return VERTICES;
}
const Index* BasePlane::GetIndices() const
{
	return INDICES;
}