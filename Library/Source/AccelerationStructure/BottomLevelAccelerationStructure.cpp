#include "AccelerationStructure\BottomLevelAccelerationStructure.h"

namespace library
{
	BottomLevelAccelerationStructure::BottomLevelAccelerationStructure(_In_ std::shared_ptr<Renderable>& pRenderable) :
		m_scratchResource(nullptr),
		m_bottomLevelAccelerationStructure(nullptr),
		m_pRenderable(pRenderable)
	{}
	HRESULT BottomLevelAccelerationStructure::Initialize(
		_In_ ID3D12Device5* pDevice,
		_In_ ID3D12GraphicsCommandList4* pCommandList
	)
	{
		HRESULT hr = S_OK;
		D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc = {//ray tracing geometry���� ����ü ����, BLAS�� ������ �����̶� �� �� ����
			.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES,// geometry�� �ﰢ��
			.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE,//geometry�� ������
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
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO bottomLevelPrebuildInfo = {};
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS bottomLevelInputs = {
			.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL,//BLAS
			.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE,
			.NumDescs = 1,
			.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY,
			.pGeometryDescs = &geometryDesc
		};
		pDevice->GetRaytracingAccelerationStructurePrebuildInfo(&bottomLevelInputs, &bottomLevelPrebuildInfo);
		if (bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes <= 0)//0����Ʈ���� ū�� �˻�
		{
			return E_FAIL;
		}
		{//scratch ���� ����
			CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
			CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(
				bottomLevelPrebuildInfo.ScratchDataSizeInBytes,
				D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
			);
			hr = pDevice->CreateCommittedResource(
				&uploadHeapProperties,
				D3D12_HEAP_FLAG_NONE,
				&bufferDesc,
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
				nullptr,
				IID_PPV_ARGS(&m_scratchResource)
			);
			if (FAILED(hr))
			{
				return hr;
			}
		}
		{//BLAS���� ����
			CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
			CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(
				bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes,
				D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
			);
			D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;//resource state�� acceleration structure

			hr = pDevice->CreateCommittedResource(
				&uploadHeapProperties,
				D3D12_HEAP_FLAG_NONE,
				&bufferDesc,
				initialResourceState,
				nullptr,
				IID_PPV_ARGS(&m_bottomLevelAccelerationStructure)
			);
			if (FAILED(hr))
			{
				return hr;
			}
		}
		// BLAS����ɼ�
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC  blasBuildDesc = {
			.DestAccelerationStructureData = m_bottomLevelAccelerationStructure->GetGPUVirtualAddress(),//BLAS���� �ּ�
			.Inputs = bottomLevelInputs,                                                                //��ǲ
			.SourceAccelerationStructureData = 0,                                                       //?
			.ScratchAccelerationStructureData = m_scratchResource->GetGPUVirtualAddress()                 //scratch���� �ּ�
		};
		pCommandList->BuildRaytracingAccelerationStructure(&blasBuildDesc, 0, nullptr);//BLAS�����
		CD3DX12_RESOURCE_BARRIER tempUAV = CD3DX12_RESOURCE_BARRIER::UAV(m_bottomLevelAccelerationStructure.Get());
		pCommandList->ResourceBarrier(1, &tempUAV);// Barrier, ����: TLAS������������ BLAS UAV���۸� �����ؼ�?
		return S_OK;
	}
	const std::shared_ptr<Renderable>& BottomLevelAccelerationStructure::GetRenderable() const
	{
		return m_pRenderable;
	}
	void BottomLevelAccelerationStructure::Update(_In_ FLOAT deltaTime)
	{}
	D3D12_GPU_VIRTUAL_ADDRESS BottomLevelAccelerationStructure::GetGPUVirtualAddress() const
	{
		return m_bottomLevelAccelerationStructure->GetGPUVirtualAddress();
	}
}