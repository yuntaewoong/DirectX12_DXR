#include "Cube\BaseCube.h"

BaseCube::BaseCube(
	_In_ XMVECTOR location,
	_In_ XMVECTOR rotation,
	_In_ XMVECTOR scale
) :
	Renderable::Renderable(location,rotation,scale)
{}
HRESULT BaseCube::Initialize(_In_ ID3D12Device* pDevice)
{
	HRESULT hr = S_OK;
	hr = Renderable::initialize(pDevice);
	if (FAILED(hr))
	{
		return hr;
	}
	return hr;
}
void BaseCube::Update(_In_ FLOAT deltaTime)
{

}
UINT BaseCube::GetNumVertices() const
{
	return ARRAYSIZE(VERTICES);
}
UINT BaseCube::GetNumIndices() const
{
	return ARRAYSIZE(INDICES);
}
const library::Vertex* BaseCube::GetVertices() const
{
	return VERTICES;
}
const library::Index* BaseCube::GetIndices() const
{
	return INDICES;
}