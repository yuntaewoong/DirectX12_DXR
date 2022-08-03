#include "AccelerationStructure\TopLevelAccelerationStructure.h"

namespace library
{
	TopLevelAccelerationStructure::TopLevelAccelerationStructure() :
		m_scratchResource(nullptr),
		m_topLevelAccelerationStructure(nullptr),
		m_instanceResource(nullptr)
	{}
    HRESULT TopLevelAccelerationStructure::Initialize(
        _In_ ID3D12Device5* pDevice,
        _In_ ID3D12GraphicsCommandList4* pCommandList,
        _In_ const std::vector<std::unique_ptr<BottomLevelAccelerationStructure>>& m_bottomLevelAccelerationStructures
    )
    {
        HRESULT hr = S_OK;
        {//blas마다 대응하는 instanceResource생성후 GPU mem copy
            std::vector<D3D12_RAYTRACING_INSTANCE_DESC> instanceDescs;
            instanceDescs.resize(m_bottomLevelAccelerationStructures.size());
            for (INT i = 0; i < instanceDescs.size(); i++)
            {
                XMFLOAT4X4 transform;
                XMStoreFloat4x4(&transform, m_bottomLevelAccelerationStructures[i]->GetRenderable()->GetWorldMatrix());
                instanceDescs[i] = {
                    .Transform = {
                        transform._11,transform._12,transform._13,transform._14,
                        transform._21,transform._22,transform._23,transform._24,
                        transform._31,transform._32,transform._33,transform._34,
                    },
                    .InstanceID = static_cast<UINT>(i),
                    .InstanceMask = 1,
                    .InstanceContributionToHitGroupIndex = 0,
                    .AccelerationStructure = m_bottomLevelAccelerationStructures[i]->GetGPUVirtualAddress()
                };
            }
            CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
            CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(instanceDescs[0]) * instanceDescs.size());
            hr = pDevice->CreateCommittedResource(
                &uploadHeapProperties,
                D3D12_HEAP_FLAG_NONE,
                &bufferDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&m_instanceResource)
            );
            void* pMappedData = nullptr;
            m_instanceResource->Map(0, nullptr, &pMappedData);
            memcpy(pMappedData, instanceDescs.data(), instanceDescs.size() * sizeof(instanceDescs[0]));
            m_instanceResource->Unmap(0, nullptr);
        }
        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS topLevelInputs = {
            .Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL,//TLAS
            .Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE,
            .NumDescs = 1,
            .DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY,
            .InstanceDescs = m_instanceResource->GetGPUVirtualAddress()
        };
        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo = {};
        pDevice->GetRaytracingAccelerationStructurePrebuildInfo(&topLevelInputs, &topLevelPrebuildInfo);//toplevelinput을 넣어주면 toplevelprebuildinfo가 나옴
        if (topLevelPrebuildInfo.ResultDataMaxSizeInBytes <= 0)//0바이트보다 큰지 검사
        {
            return E_FAIL;
        }

        {//scrath resource만들기
            CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
            CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(
                topLevelPrebuildInfo.ScratchDataSizeInBytes,
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
        {//TLAS 버퍼 만들기
            CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
            CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(
                topLevelPrebuildInfo.ResultDataMaxSizeInBytes,
                D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
            );
            D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;//resource state는 acceleration structure
            hr = pDevice->CreateCommittedResource(
                &uploadHeapProperties,
                D3D12_HEAP_FLAG_NONE,
                &bufferDesc,
                initialResourceState,
                nullptr,
                IID_PPV_ARGS(&m_topLevelAccelerationStructure)
            );
            if (FAILED(hr))
            {
                return hr;
            }
        }
        // TLAS빌드옵션
        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC tlasBuildDesc = {
            .DestAccelerationStructureData = m_topLevelAccelerationStructure->GetGPUVirtualAddress(),//TLAS버퍼 주소
            .Inputs = topLevelInputs,                                                                //인풋 
            .SourceAccelerationStructureData = 0,                                                    //?
            .ScratchAccelerationStructureData = m_scratchResource->GetGPUVirtualAddress()              //scratch버퍼 주소
        };
        pCommandList->BuildRaytracingAccelerationStructure(&tlasBuildDesc, 0, nullptr);
        return S_OK;
	}
	D3D12_GPU_VIRTUAL_ADDRESS TopLevelAccelerationStructure::GetTLASVirtualAddress() const
	{
		return m_topLevelAccelerationStructure->GetGPUVirtualAddress();
	}
	void TopLevelAccelerationStructure::Update(_In_ FLOAT deltaTime)
	{

	}
}