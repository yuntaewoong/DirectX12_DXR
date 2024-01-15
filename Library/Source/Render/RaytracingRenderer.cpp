#include "pch.h"
#include "Render\RaytracingRenderer.h"
#include "ShaderTable\ShaderTable.h"
#include "DescriptorHeap\CBVSRVUAVDescriptorHeap.h"
namespace library
{
    RaytracingRenderer::RaytracingRenderer(_In_ FLOAT aspectRatio) :
        m_renderingResources(),
        m_scene(nullptr),
        m_camera(Camera(XMVectorSet(0.0f, 3.0f, -6.0f, 0.0f),aspectRatio)),
        m_randomGenerator(),
        m_randomSampleCounter(),
        m_dxrDevice(nullptr),
        m_dxrCommandList(nullptr),
        m_raytracingPipelineStateObject(),
        m_globalRootSignature(),
        m_localRootSignature(),
        m_topLevelAccelerationStructure(),
        m_bottomLevelAccelerationStructures(std::vector<std::unique_ptr<BottomLevelAccelerationStructure>>()),
        m_missShaderTable(),
        m_hitGroupShaderTable(),
        m_realTimeRaygenShaderTable(),
        m_pathTracerRaygenShaderTable(),
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
        );//Mesh들을 관리하는 Scene초기화
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
        ImGui_ImplDX12_Init(
            m_dxrDevice.Get(),
            3,
            DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM,
            m_renderingResources.GetImguiDescriptorHeap().GetDescriptorHeap().Get(),
            m_renderingResources.GetImguiDescriptorHeap().GetDescriptorHeap()->GetCPUDescriptorHandleForHeapStart(),
            m_renderingResources.GetImguiDescriptorHeap().GetDescriptorHeap()->GetGPUDescriptorHandleForHeapStart()
        );
        hr = m_camera.Initialize(m_renderingResources.GetDevice().Get());
        if (FAILED(hr))
        {
            return hr;
        }
        hr = m_randomGenerator.Initialize(m_renderingResources.GetDevice().Get());
        if (FAILED(hr))
        {
            return hr;
        }
        hr = m_randomSampleCounter.Initialize(m_renderingResources.GetDevice().Get());
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
    void RaytracingRenderer::Render(_In_ UINT renderType)
    {
        populateCommandList(renderType);//command 기록
        m_renderingResources.ExecuteCommandList();//command queue에 담긴 command list들 실행명령(비동기)
        m_renderingResources.PresentSwapChain();
        m_renderingResources.MoveToNextFrame();
    }
    void RaytracingRenderer::Update(_In_ FLOAT deltaTime,_In_ UINT renderType)
    {
        m_camera.Update(deltaTime);
        m_randomGenerator.Update(deltaTime);
        m_randomSampleCounter.Update(deltaTime,renderType,m_camera.IsPastFrameMoved());
        m_scene->Update(deltaTime);
    }
    HRESULT RaytracingRenderer::populateCommandList(_In_ UINT renderType)
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
                static_cast<UINT>(EGlobalRootSignatureSlot::RandomConstantSlot),
                m_randomGenerator.GetConstantBuffer()->GetGPUVirtualAddress()
            );//Random CB바인딩
            pCommandList->SetComputeRootConstantBufferView(
                static_cast<UINT>(EGlobalRootSignatureSlot::PointLightConstantSlot),
                m_scene->GetPointLightsConstantBuffer()->GetGPUVirtualAddress()
            );//Point Light CB바인딩
            pCommandList->SetComputeRootConstantBufferView(
                static_cast<UINT>(EGlobalRootSignatureSlot::AreaLightConstantSlot),
                m_scene->GetAreaLightsConstantBuffer()->GetGPUVirtualAddress()
            );//Area Light CB바인딩
            pCommandList->SetComputeRootConstantBufferView(
                static_cast<UINT>(EGlobalRootSignatureSlot::RandomSampleCounterConstantSlot),
                m_randomSampleCounter.GetConstantBuffer()->GetGPUVirtualAddress()
            );//RandomSampleCounter CB바인딩
        }

        m_dxrCommandList->SetPipelineState1(m_raytracingPipelineStateObject.GetStateObject().Get());//열심히 만든 ray tracing pipeline바인딩
        
        D3D12_GPU_VIRTUAL_ADDRESS_RANGE rayGenerationShaderRecord = D3D12_GPU_VIRTUAL_ADDRESS_RANGE();
        switch(renderType)
        {
            case RayGenType::RealTime:
                rayGenerationShaderRecord = {
                    .StartAddress = m_realTimeRaygenShaderTable.GetShaderTableGPUVirtualAddress(),
                    .SizeInBytes = m_realTimeRaygenShaderTable.GetShaderTableSizeInBytes()
                };
                break;
            case RayGenType::PathTracer:
                rayGenerationShaderRecord = {
                    .StartAddress = m_pathTracerRaygenShaderTable.GetShaderTableGPUVirtualAddress(),
                    .SizeInBytes = m_pathTracerRaygenShaderTable.GetShaderTableSizeInBytes()
                };
                break;
        }
        D3D12_DISPATCH_RAYS_DESC dispatchDesc = {//RayTracing파이프라인 desc
            .RayGenerationShaderRecord = rayGenerationShaderRecord,
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
        m_dxrCommandList->DispatchRays(&dispatchDesc);// 모든 픽셀에 대해 ray generation shader실행명령
        
        ComPtr<ID3D12Resource>& currentRenderTarget = m_renderingResources.GetCurrentRenderTarget();

        D3D12_RESOURCE_BARRIER preCopyBarriers[2] = {};
        preCopyBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(
            currentRenderTarget.Get(),
            D3D12_RESOURCE_STATE_COMMON,
            D3D12_RESOURCE_STATE_COPY_DEST
        );
        preCopyBarriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(
            m_raytracingOutput.Get(),
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE
        );
        pCommandList->ResourceBarrier(ARRAYSIZE(preCopyBarriers), preCopyBarriers);
        
        pCommandList->CopyResource(currentRenderTarget.Get(), m_raytracingOutput.Get());

        D3D12_RESOURCE_BARRIER postCopyBarriers[2] = {};
        postCopyBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(
            currentRenderTarget.Get(),
            D3D12_RESOURCE_STATE_COPY_DEST, 
            D3D12_RESOURCE_STATE_RENDER_TARGET
        );
        postCopyBarriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(
            m_raytracingOutput.Get(),
            D3D12_RESOURCE_STATE_COPY_SOURCE,
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS
        );
        pCommandList->ResourceBarrier(ARRAYSIZE(postCopyBarriers), postCopyBarriers);
        
        {//imgui render
            pCommandList->SetDescriptorHeaps(1,m_renderingResources.GetImguiDescriptorHeap().GetDescriptorHeap().GetAddressOf());
            D3D12_CPU_DESCRIPTOR_HANDLE rtvCPUHandle = m_renderingResources.GetRTVDescriptorHeap().GetRTVCPUHandle(m_renderingResources.GetFrameIndex());
            pCommandList->OMSetRenderTargets(1u,&rtvCPUHandle,FALSE,nullptr);
            ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), pCommandList.Get());
        }
        D3D12_RESOURCE_BARRIER postImguiBarriers[1] = {};
        postImguiBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(
            currentRenderTarget.Get(),
            D3D12_RESOURCE_STATE_RENDER_TARGET, 
            D3D12_RESOURCE_STATE_COMMON
        );
        pCommandList->ResourceBarrier(ARRAYSIZE(postImguiBarriers), postImguiBarriers);
        

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
        {//BLAS를 이루게 되는 Mesh들 추출
            const std::vector<std::shared_ptr<Mesh>>& meshes = m_scene->GetMeshes();
            for (UINT i = 0; i < meshes.size(); i++)
            {
                m_bottomLevelAccelerationStructures.push_back(std::make_unique<BottomLevelAccelerationStructure>(meshes[i]));
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
        hr = m_realTimeRaygenShaderTable.Initialize(pDevice,m_raytracingPipelineStateObject.GetStateObject(),RAY_GEN_SHADER_NAMES[RayGenType::RealTime]);
        if (FAILED(hr))
        {
            return hr;
        }
        hr = m_pathTracerRaygenShaderTable.Initialize(pDevice,m_raytracingPipelineStateObject.GetStateObject(),RAY_GEN_SHADER_NAMES[RayGenType::PathTracer]);
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
            m_scene->GetMeshes()
        );//hit group table을 초기화 하기 위해서는 Mesh들의 정보가 필요
        if (FAILED(hr))
        {
            return hr;
        }
        return hr;
    }
    HRESULT RaytracingRenderer::createRaytacingOutputResource(_In_ HWND hWnd)
    {
        HRESULT hr = S_OK;\
        CD3DX12_RESOURCE_DESC uavDesc = CD3DX12_RESOURCE_DESC::Tex2D(
            DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM, 
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