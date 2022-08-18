#include "pch.h"
#include "AccelerationStructure\BottomLevelAccelerationStructure.h"

namespace library
{
	BottomLevelAccelerationStructure::BottomLevelAccelerationStructure(_In_ const std::shared_ptr<Renderable>& pRenderable) :
		AccelerationStructure::AccelerationStructure(),
		m_pRenderable(pRenderable),
		m_geometryDesc()
	{}
	HRESULT BottomLevelAccelerationStructure::Initialize(
		_In_ ID3D12Device5* pDevice,
		_In_ ID3D12GraphicsCommandList4* pCommandList
	)
	{
		HRESULT hr = S_OK;
		createGeometryDesc();
		hr = AccelerationStructure::initialize(pDevice, pCommandList);
		if (FAILED(hr))
		{
			return hr;
		}
		CD3DX12_RESOURCE_BARRIER tempUAV = CD3DX12_RESOURCE_BARRIER::UAV(GetAccelerationStructure().Get());
		pCommandList->ResourceBarrier(1, &tempUAV);// Barrier
		return hr;
	}
	const std::shared_ptr<Renderable>& BottomLevelAccelerationStructure::GetRenderable() const
	{
		return m_pRenderable;
	}
	void BottomLevelAccelerationStructure::Update(_In_ FLOAT deltaTime)
	{}
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS BottomLevelAccelerationStructure::createInput()
	{
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS blasInput = {
			.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL,//BLAS
			.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE,
			.NumDescs = 1,
			.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY,
			.pGeometryDescs = &m_geometryDesc
		};
		return blasInput;
	}
	void BottomLevelAccelerationStructure::createGeometryDesc()
	{
		m_geometryDesc = {//ray tracing geometry정보 구조체 정의, BLAS의 아이템 형식이라 할 수 있음
			.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES,// geometry는 삼각형
			.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE,//geometry는 불투명
			.Triangles = {
				.Transform3x4 = 0,
				.IndexFormat = DXGI_FORMAT_R16_UINT,
				.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT,
				.IndexCount = m_pRenderable->GetNumIndices(),
				.VertexCount = m_pRenderable->GetNumVertices(),
				.IndexBuffer = m_pRenderable->GetIndexBuffer()->GetGPUVirtualAddress(),
				.VertexBuffer = {
					.StartAddress = m_pRenderable->GetVertexBuffer()->GetGPUVirtualAddress(),
					.StrideInBytes = sizeof(Vertex)
				}
			}
		};
	}
}