#include "Render\RaytracingRenderer.h"
#include "ShaderTable\ShaderTable.h"

#include "CompiledShaders\BasicRayTracing.hlsl.h"
namespace library
{
    RaytracingRenderer::RaytracingRenderer() :
        m_renderingResources(),
        m_scene(nullptr),
        m_camera(Camera(XMVectorSet(0.0f, 3.0f, -6.0f, 0.0f))),
        m_dxrDevice(nullptr),
        m_dxrCommandList(nullptr),
        m_dxrStateObject(nullptr),
        m_globalRootSignature(),
        m_localRootSignature(),
        m_topLevelAccelerationStructure(),
        m_bottomLevelAccelerationStructures(std::vector<std::unique_ptr<BottomLevelAccelerationStructure>>()),
        m_descriptorHeap(nullptr),
        m_descriptorsAllocated(0u),
        m_uavHeapDescriptorSize(0u),
        m_missShaderTable(),
        m_hitGroupShaderTable(),
        m_rayGenShaderTable(),
        m_raytracingOutput(nullptr),
        m_raytracingOutputResourceUAVGpuDescriptor(),
        m_raytracingOutputResourceUAVDescriptorHeapIndex(UINT_MAX),
        m_indexBufferCpuDescriptorHandle(),
        m_indexBufferGpuDescriptorHandle(),
        m_vertexBufferCpuDescriptorHandle(),
        m_vertexBufferGpuDescriptorHandle()
    {}
	HRESULT RaytracingRenderer::Initialize(_In_ HWND hWnd)
	{
        HRESULT hr = S_OK;
        hr = m_renderingResources.Initialize(hWnd);
        if (FAILED(hr))
        {
            return hr;
        }
        hr = m_scene->Initialize(m_renderingResources.GetDevice().Get());//Renderable들을 관리하는 Scene초기화
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
        hr = createUAVDescriptorHeap();//ray tracing결과 그릴 UAV텍스처에 대한 descriptor heap만들기
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
        {//SRV생성
            const std::shared_ptr<Renderable>& tempRenderable = m_scene->GetRenderables()[0];//테스트용 렌더러블 레퍼런스 1개 가져오기
            UINT a = createBufferSRV(tempRenderable->GetIndexBuffer().Get(), &m_indexBufferCpuDescriptorHandle, &m_indexBufferGpuDescriptorHandle, tempRenderable->GetNumIndices()/2, 0);
            UINT b = createBufferSRV(tempRenderable->GetVertexBuffer().Get(), &m_vertexBufferCpuDescriptorHandle, &m_vertexBufferGpuDescriptorHandle, tempRenderable->GetNumVertices(), sizeof(Vertex));
            
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
        pCommandList->SetDescriptorHeaps(1, m_descriptorHeap.GetAddressOf());//command list에 descriptor heap을 바인딩
        pCommandList->SetComputeRootDescriptorTable(
            static_cast<UINT>(EGlobalRootSignatureSlot::OutputViewSlot),
            m_raytracingOutputResourceUAVGpuDescriptor
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

        pCommandList->SetComputeRootDescriptorTable(
            static_cast<UINT>(EGlobalRootSignatureSlot::VertexBuffersSlot),
            m_indexBufferGpuDescriptorHandle
        );//vertex,index버퍼 바인딩

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
            .Width = 1920,
            .Height = 1080,
            .Depth = 1
        };
        auto aa = m_hitGroupShaderTable.GetShaderRecordSize();
        m_dxrCommandList->SetPipelineState1(m_dxrStateObject.Get());//열심히 만든 ray tracing pipeline바인딩
        m_dxrCommandList->DispatchRays(&dispatchDesc);// 모든 픽셀에 대해 ray generation shader실행명령
        


        ComPtr<ID3D12Resource>& currentRenderTarget = m_renderingResources.GetCurrentRenderTarget();
        D3D12_RESOURCE_BARRIER preCopyBarriers[2] = {};
        preCopyBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(
            currentRenderTarget.Get(),
            D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_COPY_DEST
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
            D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PRESENT
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
        CD3DX12_STATE_OBJECT_DESC raytracingPipeline{ D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE };//내가 만들 state object는 레이트레이싱 파이프라인


        //DXIL subobject
        CD3DX12_DXIL_LIBRARY_SUBOBJECT* lib = raytracingPipeline.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();//셰이더를 wrapping하는 DXIL라이브러리 서브오브젝트 생성
        D3D12_SHADER_BYTECODE libdxil = CD3DX12_SHADER_BYTECODE(static_cast<const void*>(g_pBasicRayTracing), ARRAYSIZE(g_pBasicRayTracing));//빌드 타임 컴파일 셰이더 바이트코드 가져오기
        lib->SetDXILLibrary(&libdxil);//DXIL-Raytacing Shader 연결
        {
            lib->DefineExport(L"MyRaygenShader");//ray generation shader진입점 정의
            lib->DefineExport(L"MyClosestHitShader");//closest hit shader진입점 정의
            lib->DefineExport(L"MyMissShader");//miss shader shader진입점 정의

            lib->DefineExport(L"MyShadowRayClosestHitShader");//ShadowRay closest hit shader진입점 정의
            lib->DefineExport(L"MyShadowRayMissShader");// ShadowRay miss shader shader진입점 정의
        }

        {//radiance hit group 서브 오브젝트 생성
            CD3DX12_HIT_GROUP_SUBOBJECT* hitGroup = raytracingPipeline.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
            hitGroup->SetClosestHitShaderImport(L"MyClosestHitShader");//히트그룹과 연결될 셰이더진입점
            hitGroup->SetHitGroupExport(L"MyHitGroup");//히트 그룹 수출
            hitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);//이 히트그룹은 삼각형
        }
        {//Shadow hit group 서브 오브젝트 생성
            CD3DX12_HIT_GROUP_SUBOBJECT* hitGroup = raytracingPipeline.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
            hitGroup->SetClosestHitShaderImport(L"MyShadowRayClosestHitShader");//히트그룹과 연결될 셰이더진입점
            hitGroup->SetHitGroupExport(L"MyShadowRayHitGroup");//히트 그룹 수출
            hitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);//이 히트그룹은 삼각형
        }
        {//Shader Config서브오브젝트 생성
            CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT* shaderConfig = raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
            UINT payloadSize = max(sizeof(RayPayload), sizeof(ShadowRayPayload));   //  둘중 큰값을 할당
            UINT attributeSize = sizeof(XMFLOAT2); //barycentrics
            shaderConfig->Config(payloadSize, attributeSize);// payload, attribute사이즈 정의(셰이더에서 인자로 사용됨)
        }
        
        {//미리 만들어둔 local root signature로 서브오브젝트 만들어 파이프라인에 적용
            CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT* localRootSignature = raytracingPipeline.CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
            localRootSignature->SetRootSignature(m_localRootSignature.GetRootSignature().Get());
            CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT* rootSignatureAssociation = raytracingPipeline.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
            rootSignatureAssociation->SetSubobjectToAssociate(*localRootSignature);
            rootSignatureAssociation->AddExport(L"MyHitGroup");//My Hit Group에서 사용하겠다
        }

        //모든 Shader에서 사용할 gloabl root signature서브 오브젝트 적용
        CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT* globalRootSignature = raytracingPipeline.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();// 글로벌 루트시그니처 서브오브젝트생성
        globalRootSignature->SetRootSignature(m_globalRootSignature.GetRootSignature().Get());
        //파이프라인 옵션
        CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT* pipelineConfig = raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();// 파이프라인 config 서브오브젝트 생성 
        UINT maxRecursionDepth = MAX_RECURSION_DEPTH;//2번만 recursion 하겠다(1: 기본 shading용, 2:shadow ray용)
        pipelineConfig->Config(maxRecursionDepth);//적용


        //재료가지고 최종 StateObejct생성
        HRESULT hr = S_OK;
        hr = m_dxrDevice->CreateStateObject(raytracingPipeline, IID_PPV_ARGS(&m_dxrStateObject));//레이트레이싱 커스텀 파이프라인 생성
        if (FAILED(hr))
        {
            return hr;
        }
        return hr;
    }
    HRESULT RaytracingRenderer::createUAVDescriptorHeap()
    {
        HRESULT hr = S_OK;
        D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {
            .Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,//CBV,SRV,UAV타입이다
            .NumDescriptors = 3u,
            .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,//shader에서 볼수있다
            .NodeMask = 0
        };
        hr = m_renderingResources.GetDevice()->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&m_descriptorHeap));//create
        if (FAILED(hr))
        {
            return hr;
        }
        m_uavHeapDescriptorSize = m_renderingResources.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);//cpu별로 상이한 descriptor사이즈 가져오기
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
        hr = m_rayGenShaderTable.Initialize(pDevice,m_dxrStateObject);
        if (FAILED(hr))
        {
            return hr;
        }
        hr = m_missShaderTable.Initialize(pDevice,m_dxrStateObject);
        if (FAILED(hr))
        {
            return hr;
        }
        hr = m_hitGroupShaderTable.Initialize(pDevice,m_dxrStateObject,m_scene->GetRenderables());//hit group table을 초기화 하기 위해서는 renderable들의 정보가 필요
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
        hr = m_renderingResources.GetDevice()->CreateCommittedResource(//결과물 UAV버퍼 생성
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
        D3D12_CPU_DESCRIPTOR_HANDLE descriptorHeapCpuBase = m_descriptorHeap->GetCPUDescriptorHandleForHeapStart();
        if (m_raytracingOutputResourceUAVDescriptorHeapIndex >= m_descriptorHeap->GetDesc().NumDescriptors)
        {
            m_raytracingOutputResourceUAVDescriptorHeapIndex = m_descriptorsAllocated++;
        }
        D3D12_CPU_DESCRIPTOR_HANDLE uavDescriptorHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(
            descriptorHeapCpuBase, 
            m_raytracingOutputResourceUAVDescriptorHeapIndex,
            m_uavHeapDescriptorSize
        );
        D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {
            .ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D
        };
        m_renderingResources.GetDevice()->CreateUnorderedAccessView(m_raytracingOutput.Get(), nullptr, &UAVDesc, uavDescriptorHandle);
        m_raytracingOutputResourceUAVGpuDescriptor = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_descriptorHeap->GetGPUDescriptorHandleForHeapStart(), m_raytracingOutputResourceUAVDescriptorHeapIndex, m_uavHeapDescriptorSize);
        return hr;
    }
    UINT RaytracingRenderer::createBufferSRV(_In_ ID3D12Resource* buffer, _Out_ D3D12_CPU_DESCRIPTOR_HANDLE* cpuDescriptorHandle, _Out_ D3D12_GPU_DESCRIPTOR_HANDLE* gpuDescriptorHandle, _In_ UINT numElements, _In_ UINT elementSize)
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Buffer.NumElements = numElements;
        if (elementSize == 0)
        {
            srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
            srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
            srvDesc.Buffer.StructureByteStride = 0;
        }
        else
        {
            srvDesc.Format = DXGI_FORMAT_UNKNOWN;
            srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
            srvDesc.Buffer.StructureByteStride = elementSize;
        }
        UINT descriptorIndex = UINT_MAX;
        auto descriptorHeapCpuBase = m_descriptorHeap->GetCPUDescriptorHandleForHeapStart();
        if (descriptorIndex >= m_descriptorHeap->GetDesc().NumDescriptors)
        {
            descriptorIndex = m_descriptorsAllocated++;
        }
        *cpuDescriptorHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(descriptorHeapCpuBase, descriptorIndex, m_uavHeapDescriptorSize);
        
        m_renderingResources.GetDevice()->CreateShaderResourceView(buffer, &srvDesc, *cpuDescriptorHandle);
        *gpuDescriptorHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_descriptorHeap->GetGPUDescriptorHandleForHeapStart(), descriptorIndex, m_uavHeapDescriptorSize);
        return descriptorIndex;
    }

}