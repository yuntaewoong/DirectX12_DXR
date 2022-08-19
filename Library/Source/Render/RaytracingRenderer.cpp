#include "pch.h"
#include "Render\RaytracingRenderer.h"
#include "ShaderTable\ShaderTable.h"
#include "DescriptorHeap\CBVSRVUAVDescriptorHeap.h"
namespace library
{
    RaytracingRenderer::RaytracingRenderer() :
        m_renderingResources(),
        m_scene(nullptr),
        m_camera(Camera(XMVectorSet(0.0f, 3.0f, -6.0f, 0.0f))),
        m_dxrDevice(nullptr),
        m_dxrCommandList(nullptr),
        m_raytracingPipelineStateObject(),
        m_globalRootSignature(),
        m_localRootSignature(),
        m_topLevelAccelerationStructure(),
        m_bottomLevelAccelerationStructures(std::vector<std::unique_ptr<BottomLevelAccelerationStructure>>()),
        m_missShaderTable(),
        m_hitGroupShaderTable(),
        m_rayGenShaderTable(),
        m_raytracingOutput(nullptr),
        m_raytracingOutputGPUHandle()
    {}
	HRESULT RaytracingRenderer::Initialize(_In_ HWND hWnd)
	{
        HRESULT hr = S_OK;
        hr = m_renderingResources.Initialize(hWnd);
        if (FAILED(hr))
        {
            return hr;
        }
        hr = m_scene->Initialize(
            m_renderingResources.GetDevice(),
            m_renderingResources.GetCommandQueue(),
            m_renderingResources.GetCBVSRVUAVDescriptorHeap()
        );//Renderable들을 관리하는 Scene초기화
        if (FAILED(hr))
        {
            return hr;
        }
        //이하는 RayTracing용 초기화
        
        hr = createRaytracingInterfaces();//device,commandList ray tracing지원용으로 query
        if (FAILED(hr))
        {
            return hr;
        }
        hr = createRaytracingRootSignature();//ray tracing에서 사용할 global,local root signature생성
        if (FAILED(hr))
        {
            return hr;
        }
        hr = createRaytracingPipelineStateObject();//ray tracing pipeline생성
        if (FAILED(hr))
        {
            return hr;
        }
        hr = createAccelerationStructure();//ray tracing할 geometry들 TLAS,BLAS자료구조로 잘 정리하기
        if (FAILED(hr))
        {
            return hr;
        }
        hr = createShaderTable();// shader table만들기
        if (FAILED(hr))
        {
            return hr;
        }
        hr = createRaytacingOutputResource(hWnd);//output UAV만들기
        if (FAILED(hr))
        {
            return hr;
        }

        hr = m_camera.Initialize(m_renderingResources.GetDevice().Get());
        if (FAILED(hr))
        {
            return hr;
        }
        hr = m_renderingResources.WaitForGPU();
        if (FAILED(hr))
        {
            return hr;
        }
        return hr;
	}
    void RaytracingRenderer::HandleInput(_In_ const DirectionsInput& directions, _In_ const MouseRelativeMovement& mouseRelativeMovement, _In_ FLOAT deltaTime)
    {
        m_camera.HandleInput(directions, mouseRelativeMovement, deltaTime);
    }
    void RaytracingRenderer::SetMainScene(_In_ const std::shared_ptr<Scene>& pScene)
    {
        m_scene = pScene;
    }
    void RaytracingRenderer::Render()
    {
        populateCommandList();//command 기록
        m_renderingResources.ExecuteCommandList();//command queue에 담긴 command list들 실행명령(비동기)
        m_renderingResources.PresentSwapChain();
        m_renderingResources.MoveToNextFrame();
    }
    void RaytracingRenderer::Update(_In_ FLOAT deltaTime)
    {
        m_camera.Update(deltaTime);
        m_scene->Update(deltaTime);
    }
    HRESULT RaytracingRenderer::populateCommandList()
    {
        ComPtr<ID3D12GraphicsCommandList>& pCommandList = m_renderingResources.GetCommandList();
        //command list allocator특: gpu동작 끝나야 Reset가능(펜스로 동기화해라)
        HRESULT hr = S_OK;
        hr = m_renderingResources.ResetCommandAllocator();
        if (FAILED(hr))
        {
            return hr;
        }
        hr = m_renderingResources.ResetCommandList();
        if (FAILED(hr))
        {
            return hr;
        }
        pCommandList->SetComputeRootSignature(m_globalRootSignature.GetRootSignature().Get());//compute shader의 루트 시그니처 바인딩
        pCommandList->SetDescriptorHeaps(1, m_renderingResources.GetCBVSRVUAVDescriptorHeap().GetDescriptorHeap().GetAddressOf());//command list에 descriptor heap을 바인딩
        
        {//Global root signature바인딩
            pCommandList->SetComputeRootDescriptorTable(
                static_cast<UINT>(EGlobalRootSignatureSlot::OutputViewSlot),
                m_raytracingOutputGPUHandle
            );//아웃풋 UAV텍스쳐 바인딩
            pCommandList->SetComputeRootShaderResourceView(
                static_cast<UINT>(EGlobalRootSignatureSlot::AccelerationStructureSlot),
                m_topLevelAccelerationStructure.GetAccelerationStructure()->GetGPUVirtualAddress()
            );//TLAS 바인딩
            pCommandList->SetComputeRootConstantBufferView(
                static_cast<UINT>(EGlobalRootSignatureSlot::CameraConstantSlot),
                m_camera.GetConstantBuffer()->GetGPUVirtualAddress()
            );//Camera CB바인딩
            pCommandList->SetComputeRootConstantBufferView(
                static_cast<UINT>(EGlobalRootSignatureSlot::LightConstantSlot),
                m_scene->GetPointLightsConstantBuffer()->GetGPUVirtualAddress()
            );//Light CB바인딩
        }

        
        D3D12_DISPATCH_RAYS_DESC dispatchDesc = {//RayTracing파이프라인 desc
            .RayGenerationShaderRecord = {
                .StartAddress = m_rayGenShaderTable.GetShaderTableGPUVirtualAddress(),
                .SizeInBytes = m_rayGenShaderTable.GetShaderTableSizeInBytes()
            },
            .MissShaderTable = {
                .StartAddress = m_missShaderTable.GetShaderTableGPUVirtualAddress(),
                .SizeInBytes = m_missShaderTable.GetShaderTableSizeInBytes(),
                .StrideInBytes = m_missShaderTable.GetShaderTableStrideInBytes()
            },
            .HitGroupTable = {
                .StartAddress = m_hitGroupShaderTable.GetShaderTableGPUVirtualAddress(),
                .SizeInBytes = m_hitGroupShaderTable.GetShaderTableSizeInBytes(),
                .StrideInBytes = m_hitGroupShaderTable.GetShaderTableStrideInBytes()
            },
            .Width = m_renderingResources.GetWidth(),
            .Height = m_renderingResources.GetHeight(),
            .Depth = 1
        };
        m_dxrCommandList->SetPipelineState1(m_raytracingPipelineStateObject.GetStateObject().Get());//열심히 만든 ray tracing pipeline바인딩
        m_dxrCommandList->DispatchRays(&dispatchDesc);// 모든 픽셀에 대해 ray generation shader실행명령
        
        ComPtr<ID3D12Resource>& currentRenderTarget = m_renderingResources.GetCurrentRenderTarget();
        D3D12_RESOURCE_BARRIER preCopyBarriers[2] = {};
        preCopyBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(
            currentRenderTarget.Get(),
            D3D12_RESOURCE_STATE_COMMON,
            D3D12_RESOURCE_STATE_COPY_DEST
        );//Render Target을 Copy목적지로 전이
        preCopyBarriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(
            m_raytracingOutput.Get(),
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE
        );//UAV에서 Copy
        pCommandList->ResourceBarrier(ARRAYSIZE(preCopyBarriers), preCopyBarriers);

        pCommandList->CopyResource(currentRenderTarget.Get(), m_raytracingOutput.Get());

        D3D12_RESOURCE_BARRIER postCopyBarriers[2] = {};
        postCopyBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(
            currentRenderTarget.Get(),
            D3D12_RESOURCE_STATE_COPY_DEST, 
            D3D12_RESOURCE_STATE_COMMON
        );//Render Target을 present단계로 만들기
        postCopyBarriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(
            m_raytracingOutput.Get(),
            D3D12_RESOURCE_STATE_COPY_SOURCE,
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS
        );//UAV를 다시 UA로 만들기
        pCommandList->ResourceBarrier(ARRAYSIZE(postCopyBarriers), postCopyBarriers);
        return S_OK;
    }
    
    HRESULT RaytracingRenderer::createRaytracingInterfaces()
    {
        HRESULT hr = S_OK;
        hr = m_renderingResources.GetDevice()->QueryInterface(IID_PPV_ARGS(&m_dxrDevice));
        if (FAILED(hr))
        {
            return hr;
        }
        hr = m_renderingResources.GetCommandList()->QueryInterface(IID_PPV_ARGS(&m_dxrCommandList));
        if (FAILED(hr))
        {
            return hr;
        }
        return hr;
    }
    HRESULT RaytracingRenderer::createRaytracingRootSignature()
    {
        HRESULT hr = S_OK;
        hr = m_globalRootSignature.Initialize(m_renderingResources.GetDevice().Get());
        if (FAILED(hr))
        {
            return hr;
        }
        hr = m_localRootSignature.Initialize(m_renderingResources.GetDevice().Get());
        if (FAILED(hr))
        {
            return hr;
        }
        return hr;
    }
    HRESULT RaytracingRenderer::createRaytracingPipelineStateObject()
    {
        HRESULT hr = S_OK;
        hr = m_raytracingPipelineStateObject.Initialize(
            m_dxrDevice,
            m_localRootSignature.GetRootSignature(),
            m_globalRootSignature.GetRootSignature()
        );
        if (FAILED(hr))
        {
            return hr;
        }
        return hr;
    }
    HRESULT RaytracingRenderer::createAccelerationStructure()
    {
        HRESULT hr = S_OK;
        hr = m_renderingResources.ResetCommandList();
        if (FAILED(hr))
        {
            return hr;
        }
        {//AS를 이루게 되는 Renderable들 추출
            const std::vector<std::shared_ptr<Renderable>>& renderables = m_scene->GetRenderables();
            for (UINT i = 0; i < renderables.size(); i++)
            {
                m_bottomLevelAccelerationStructures.push_back(std::make_unique<BottomLevelAccelerationStructure>(renderables[i]));
            }
        }
        {//TLAS,BLAS초기화(command List매핑)
            for (auto& iBlas : m_bottomLevelAccelerationStructures)
            {
                hr = iBlas->Initialize(m_dxrDevice.Get(), m_dxrCommandList.Get());
                if (FAILED(hr))
                {
                    return hr;
                }
            }
            hr = m_topLevelAccelerationStructure.Initialize(m_dxrDevice.Get(), m_dxrCommandList.Get(), m_bottomLevelAccelerationStructures);
            if (FAILED(hr))
            {
                return hr;
            }
        }
        m_renderingResources.ExecuteCommandList();//TLAS,BLAS만드는 command list실행
        {//command list 실행완료 대기
            hr = m_renderingResources.WaitForGPU();
            if (FAILED(hr))
            {
                return hr;
            }
        }
        return hr;
    }
    HRESULT RaytracingRenderer::createShaderTable()
    {
        HRESULT hr = S_OK;
        ComPtr<ID3D12Device> pDevice = m_renderingResources.GetDevice();
        hr = m_rayGenShaderTable.Initialize(pDevice,m_raytracingPipelineStateObject.GetStateObject());
        if (FAILED(hr))
        {
            return hr;
        }
        hr = m_missShaderTable.Initialize(pDevice, m_raytracingPipelineStateObject.GetStateObject());
        if (FAILED(hr))
        {
            return hr;
        }
        hr = m_hitGroupShaderTable.Initialize(
            pDevice,
            m_raytracingPipelineStateObject.GetStateObject(),
            m_scene->GetRenderables()
        );//hit group table을 초기화 하기 위해서는 renderable들의 정보가 필요
        if (FAILED(hr))
        {
            return hr;
        }
        return hr;
    }
    HRESULT RaytracingRenderer::createRaytacingOutputResource(_In_ HWND hWnd)
    {
        HRESULT hr = S_OK;
        //format은 Swap chain의 format과 같아야함!
        CD3DX12_RESOURCE_DESC uavDesc = CD3DX12_RESOURCE_DESC::Tex2D(
            DXGI_FORMAT_R8G8B8A8_UNORM, 
            m_renderingResources.GetWidth(),
            m_renderingResources.GetHeight(),
            1, 1, 1, 0, 
            D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
        );
        CD3DX12_HEAP_PROPERTIES defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        hr = m_renderingResources.GetDevice()->CreateCommittedResource(//결과물 UA버퍼 생성
            &defaultHeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &uavDesc, 
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
            nullptr,
            IID_PPV_ARGS(&m_raytracingOutput)
        );
        if (FAILED(hr))
        {
            return hr;
        }
        D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {
            .ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D
        };
        CBVSRVUAVDescriptorHeap& descriptorHeap =  m_renderingResources.GetCBVSRVUAVDescriptorHeap();

        hr = descriptorHeap.CreateUAV(// UA버퍼에 대한 UAV생성
            m_renderingResources.GetDevice(),
            m_raytracingOutput,
            UAVDesc,
            &m_raytracingOutputGPUHandle
        );
        if (FAILED(hr))
        {
            return hr;
        }
        return hr;
    }
}