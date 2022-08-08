#include "Render/Renderer.h"
#include "ShaderTable\ShaderTable.h"
#include "CompiledShaders\BasicVertexShader.hlsl.h"
#include "CompiledShaders\BasicPixelShader.hlsl.h"
#include "CompiledShaders\BasicRayTracing.hlsl.h"
namespace library
{
    Renderer::Renderer() :
        m_scene(nullptr),
        m_dxgiFactory(nullptr),
        m_swapChain(nullptr),
        m_device(nullptr),
        m_renderTargets{ nullptr,nullptr },
        m_commandAllocator(),
        m_commandQueue(nullptr),
        m_rtvHeap(nullptr),
        m_commandList(nullptr),
        m_camera(std::make_unique<Camera>(XMVectorSet(0.0f, 3.0f, -6.0f, 0.0f))),
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
        m_cubeCB(),
        m_missShaderTable(nullptr),
        m_hitGroupShaderTable(nullptr),
        m_rayGenShaderTable(nullptr),
        m_raytracingOutput(nullptr),
        m_raytracingOutputResourceUAVGpuDescriptor(),
        m_raytracingOutputResourceUAVDescriptorHeapIndex(UINT_MAX),
        m_rtvDescriptorSize(0u),
        m_frameIndex(0u),
        m_fenceEvent(),
        m_fence(nullptr),
        m_fenceValues(),
        m_indexBufferCpuDescriptorHandle(),
        m_indexBufferGpuDescriptorHandle(),
        m_vertexBufferCpuDescriptorHandle(),
        m_vertexBufferGpuDescriptorHandle()
    {}
	HRESULT Renderer::Initialize(_In_ HWND hWnd)
	{
        HRESULT hr = S_OK;
        UINT dxgiFactoryFlags = 0;
#if defined(_DEBUG)//디버그 빌드시
        {
            ComPtr<ID3D12Debug> debugController(nullptr);
            if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
            {
                debugController->EnableDebugLayer();
                dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
            }
        }
#endif
        hr = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&m_dxgiFactory));
        if (FAILED(hr))
        {
            return hr;
        }
        hr = createDevice();
        if (FAILED(hr))
        {
            return hr;
        }
        hr = createCommandQueue();
        if (FAILED(hr))
        {
            return hr;
        }
        hr = createSwapChain(hWnd);
        if (FAILED(hr))
        {
            return hr;
        }
        m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
        hr = createRenderTargetView();
        if (FAILED(hr))
        {
            return hr;
        }
        hr = createCommandAllocator();
        if (FAILED(hr))
        {
            return hr;
        }
        {//commandList 만들기
            hr = m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator[0].Get(), nullptr, IID_PPV_ARGS(&m_commandList));
            if (FAILED(hr))
            {
                return hr;
            }

            hr = m_commandList->Close();
            if (FAILED(hr))
            {
                return hr;
            }
        }
        {//fence만들기
            hr = m_device->CreateFence(m_fenceValues[m_frameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
            if (FAILED(hr))
            {
                return hr;
            }
            m_fenceValues[m_frameIndex]++;

            m_fenceEvent.Attach(CreateEvent(nullptr, FALSE, FALSE, nullptr));
            if (m_fenceEvent == nullptr)
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
                if (FAILED(hr))
                {
                    return hr;
                }
            }
        }
        hr = m_scene->Initialize(m_device.Get());//Renderable들을 관리하는 Scene초기화
        if (FAILED(hr))
        {
            return hr;
        }
        //이하는 RayTracing용 초기화
        ComPtr<IDXGIAdapter1> tempAdapter(nullptr);
        ComPtr<IDXGIFactory1> tempFactory(nullptr);
        getHardwareAdapter(m_dxgiFactory.Get(), tempAdapter.GetAddressOf(), false);
        if (!isDeviceSupportRayTracing(tempAdapter.Get()))
        {
            return E_FAIL;//레이트레이싱 지원 X
        }
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

        hr = m_camera->Initialize(m_device.Get());
        if (FAILED(hr))
        {
            return hr;
        }

        hr = waitForGPU();
        if (FAILED(hr))
        {
            return hr;
        }
        return hr;
	}
    void Renderer::HandleInput(_In_ const DirectionsInput& directions, _In_ const MouseRelativeMovement& mouseRelativeMovement, _In_ FLOAT deltaTime)
    {
        m_camera->HandleInput(directions, mouseRelativeMovement, deltaTime);
    }
    void Renderer::SetMainScene(_In_ std::shared_ptr<Scene>& pScene)
    {
        m_scene = pScene;
    }
    void Renderer::Render()
    {
        populateCommandList();//command 기록
        m_commandList->Close();//command list작성 종료
        ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
        m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);//command queue에 담긴 command list들 실행명령(비동기)
        
        m_swapChain->Present(1,0);//백버퍼 교체
        moveToNextFrame();
    }
    void Renderer::Update(_In_ FLOAT deltaTime)
    {
        m_camera->Update(deltaTime);
        m_scene->Update(deltaTime);
    }
    /*
       하드웨어 어뎁터 가져오기
    */
    HRESULT Renderer::getHardwareAdapter(
        _In_ IDXGIFactory1* pFactory,
        _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter,
        bool requestHighPerformanceAdapter)
    {
        *ppAdapter = nullptr;
        ComPtr<IDXGIAdapter1> adapter(nullptr);
        ComPtr<IDXGIFactory6> factory6(nullptr);
        if (FAILED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6))))// dxgi factory6를 지원하는가?
        {
            return E_FAIL;
        }
        for (//여러 gpu adapter들 순회
            UINT adapterIndex = 0;
            SUCCEEDED(factory6->EnumAdapterByGpuPreference(
                adapterIndex,
                requestHighPerformanceAdapter == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED,
                IID_PPV_ARGS(&adapter)));
            ++adapterIndex)
        {
            DXGI_ADAPTER_DESC1 desc = {
                .Description = {},
                .VendorId = 0u,
                .DeviceId = 0u,
                .SubSysId = 0u,
                .Revision = 0u,
                .DedicatedVideoMemory = 0ul,
                .DedicatedSystemMemory = 0ul,
                .SharedSystemMemory = 0ul,
                .AdapterLuid = {
                    .LowPart = 0,
                    .HighPart = 0l
                },
                .Flags = 0u
            };
            adapter->GetDesc1(&desc);//어뎁터 정보 조회
            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)//소프트웨어 어댑터는 취급x
            {
                continue;
            }
            if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
            {
                return S_OK;
            }
        }
        return E_FAIL;
    }
    HRESULT Renderer::populateCommandList()
    {
        
        //command list allocator특: gpu동작 끝나야 Reset가능(펜스로 동기화해라)
        HRESULT hr = S_OK;
        hr = m_commandAllocator[m_frameIndex]->Reset();
        if (FAILED(hr))
        {
            return hr;
        }
        hr = m_commandList->Reset(m_commandAllocator[m_frameIndex].Get(), nullptr);
        if (FAILED(hr))
        {
            return hr;
        }
        m_commandList->SetComputeRootSignature(m_globalRootSignature.GetRootSignature().Get());//compute shader의 루트 시그니처 바인딩
        
        

        m_commandList->SetDescriptorHeaps(1, m_descriptorHeap.GetAddressOf());//command list에 cpu descriptor heap을 바인딩

        m_commandList->SetComputeRootDescriptorTable(
            static_cast<UINT>(EGlobalRootSignatureSlot::OutputViewSlot),
            m_raytracingOutputResourceUAVGpuDescriptor
        );//아웃풋 UAV텍스쳐 바인딩

        m_commandList->SetComputeRootShaderResourceView(
            static_cast<UINT>(EGlobalRootSignatureSlot::AccelerationStructureSlot),
            m_topLevelAccelerationStructure.GetAccelerationStructure()->GetGPUVirtualAddress()
        );//TLAS 바인딩
        
        m_commandList->SetComputeRootConstantBufferView(
            static_cast<UINT>(EGlobalRootSignatureSlot::CameraConstantSlot),
            m_camera->GetConstantBuffer()->GetGPUVirtualAddress()
        );//Camera CB바인딩

        m_commandList->SetComputeRootConstantBufferView(
            static_cast<UINT>(EGlobalRootSignatureSlot::LightConstantSlot),
            m_scene->GetPointLightsConstantBuffer()->GetGPUVirtualAddress()
        );//Light CB바인딩

        m_commandList->SetComputeRootDescriptorTable(
            static_cast<UINT>(EGlobalRootSignatureSlot::VertexBuffersSlot),
            m_indexBufferGpuDescriptorHandle
        );//vertex,index버퍼 바인딩


        D3D12_DISPATCH_RAYS_DESC dispatchDesc = {//RayTracing파이프라인 desc
            .RayGenerationShaderRecord = {
                .StartAddress = m_rayGenShaderTable->GetGPUVirtualAddress(),
                .SizeInBytes = m_rayGenShaderTable->GetDesc().Width
            },
            .MissShaderTable = {
                .StartAddress = m_missShaderTable->GetGPUVirtualAddress(),
                .SizeInBytes = m_missShaderTable->GetDesc().Width,
                .StrideInBytes = m_missShaderTable->GetDesc().Width
            },
            .HitGroupTable = {
                .StartAddress = m_hitGroupShaderTable->GetGPUVirtualAddress(),
                .SizeInBytes = m_hitGroupShaderTable->GetDesc().Width,
                .StrideInBytes = m_hitGroupShaderTable->GetDesc().Width /2
            },
            .Width = 1920,
            .Height = 1080,
            .Depth = 1
        };
        m_dxrCommandList->SetPipelineState1(m_dxrStateObject.Get());//열심히 만든 ray tracing pipeline바인딩
        m_dxrCommandList->DispatchRays(&dispatchDesc);// 모든 픽셀에 대해 ray generation shader실행명령
        



        D3D12_RESOURCE_BARRIER preCopyBarriers[2] = {};
        preCopyBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(
            m_renderTargets[m_frameIndex].Get(),
            D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_COPY_DEST
        );//Render Target을 Copy목적지로 전이
        preCopyBarriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(
            m_raytracingOutput.Get(),
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE
        );//UAV에서 Copy
        m_commandList->ResourceBarrier(ARRAYSIZE(preCopyBarriers), preCopyBarriers);

        m_commandList->CopyResource(m_renderTargets[m_frameIndex].Get(), m_raytracingOutput.Get());

        D3D12_RESOURCE_BARRIER postCopyBarriers[2] = {};
        postCopyBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(
            m_renderTargets[m_frameIndex].Get(),
            D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PRESENT
        );//Render Target을 present단계로 만들기
        postCopyBarriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(
            m_raytracingOutput.Get(),
            D3D12_RESOURCE_STATE_COPY_SOURCE,
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS
        );//UAV를 다시 UA로 만들기
        m_commandList->ResourceBarrier(ARRAYSIZE(postCopyBarriers), postCopyBarriers);
        
        return S_OK;
    }
    void Renderer::getWindowWidthHeight(_In_ HWND hWnd, _Out_ PUINT pWidth, _Out_ PUINT pHeight)
    {
        *pWidth = 0u;
        *pHeight = 0u;
        RECT rc = {
            .left = 0u,
            .top = 0u,
            .right = 0u,
            .bottom = 0u
        };
        GetClientRect(hWnd, &rc);
        *pWidth = static_cast<UINT>(rc.right - rc.left);
        *pHeight = static_cast<UINT>(rc.bottom - rc.top);
    }
    HRESULT Renderer::waitForGPU() noexcept
    {
        HRESULT hr = S_OK;
        if (m_commandQueue && m_fence && m_fenceEvent.IsValid())
        {
            // Schedule a Signal command in the GPU queue.
            UINT64 fenceValue = m_fenceValues[m_frameIndex];
            hr = m_commandQueue->Signal(m_fence.Get(), fenceValue);//command queue 작업이 끝나면 fence값 fenceValue로 업데이트 해주세요 라는 뜻
            if (FAILED(hr))
            {
                return hr;
            }   // Wait until the Signal has been processed.
            hr = m_fence->SetEventOnCompletion(fenceValue, m_fenceEvent.Get());//m_fence 개체가 fence값으로 업데이트가 완료되면 m_fenceEvent이벤트 signal
            if (FAILED(hr))
            {
                return hr;
            }
            WaitForSingleObjectEx(m_fenceEvent.Get(), INFINITE, FALSE);//m_fenceEvent가 signal될때까지 본 스레드 blocking
            // Increment the fence value for the current frame.
            m_fenceValues[m_frameIndex]++;
            return hr;
        }
        return E_FAIL;
        
    }
    HRESULT Renderer::moveToNextFrame()
    {
        HRESULT hr = S_OK;
        // Schedule a Signal command in the queue.
        const UINT64 currentFenceValue = m_fenceValues[m_frameIndex];
        
        hr = m_commandQueue->Signal(m_fence.Get(), currentFenceValue);

        // Update the back buffer index.
        m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

        // If the next frame is not ready to be rendered yet, wait until it is ready.
        if (m_fence->GetCompletedValue() < m_fenceValues[m_frameIndex])
        {
            hr = m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent.Get());
            if (FAILED(hr))
            {
                return hr;
            }
            WaitForSingleObjectEx(m_fenceEvent.Get(), INFINITE, FALSE);
        }
        // Set the fence value for the next frame.
        m_fenceValues[m_frameIndex] = currentFenceValue + 1;
        return hr;
    }
    HRESULT Renderer::createDevice()//d3d device생성
    {
        HRESULT hr = S_OK;
        ComPtr<IDXGIAdapter1> hardwareAdapter(nullptr);
        getHardwareAdapter(m_dxgiFactory.Get(), &hardwareAdapter);
        hr = D3D12CreateDevice(
            hardwareAdapter.Get(),
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&m_device)
        );
        if (FAILED(hr))
        {
            return hr;
        }
        return hr;
    }
    HRESULT Renderer::createCommandQueue()//커맨드 큐 생성(커맨드 큐는 커맨드 리스트들의 큐임)
    {
        HRESULT hr = S_OK;
        D3D12_COMMAND_QUEUE_DESC queueDesc = {
            .Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
            .Priority = 0,
            .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
            .NodeMask = 0u
        };
        hr = m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue));
        if (FAILED(hr))
        {
            return hr;
        }
        return hr;
    }
    HRESULT Renderer::createSwapChain(_In_ HWND hWnd)//스왑체인 생성
    {
        HRESULT hr = S_OK;
        UINT width = 0u;
        UINT height = 0u;
        getWindowWidthHeight(hWnd, &width, &height);
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {
            .Width = width,                                 //가로
            .Height = height,                               //세로
            .Format = DXGI_FORMAT_R8G8B8A8_UNORM,           //각 픽셀에 담을 데이터포맷
            .Stereo = false,                                //풀 스크린인가?
            .SampleDesc = {
                .Count = 1u,
                .Quality = 0u
            },
            .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT, //백버퍼의 cpu 접근 옵션
            .BufferCount = FRAME_COUNT,                     //버퍼 개수
            .Scaling = DXGI_SCALING_NONE,                   //스케일링 지원여부
            .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,    //스왑방식
            .AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED,       //alpha값 사용여부
            .Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING                              //각종 flag
        };
        ComPtr<IDXGISwapChain1> swapChain(nullptr);
        hr = m_dxgiFactory->CreateSwapChainForHwnd(
            m_commandQueue.Get(),
            hWnd,
            &swapChainDesc,
            nullptr,
            nullptr,
            &swapChain
        );
        if (FAILED(hr))
        {
            return hr;
        }
        hr = m_dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);//alt + enter로 전체화면하는거 막기
        if (FAILED(hr))
        {
            return hr;
        }
        hr = swapChain.As(&m_swapChain);
        if (FAILED(hr))
        {
            return hr;
        }
        return hr;
    }
    HRESULT Renderer::createRenderTargetView()
    {
        HRESULT hr = S_OK;
        {//RTV용 descriptor heap 생성
            D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {
                .Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,     //렌더타겟뷰 descriptor heap
                .NumDescriptors = FRAME_COUNT,              //FRAME개수 만큼
                .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
                .NodeMask = 0u
            };
            hr = m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap));
            if (FAILED(hr))
            {
                return hr;
            }
            m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);//descriptor heap 각 요소 사이즈(운영체제마다 상이)
        }
        {//RTV생성
            CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());//첫번째 descriptor가져오기(iterator로 사용됨)
            for (UINT n = 0; n < FRAME_COUNT; n++)//프레임 개수만큼
            {
                hr = m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n]));//스왑체인에서 버퍼 가져오기
                if (FAILED(hr))
                {
                    return hr;
                }
                m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
                rtvHandle.Offset(1, m_rtvDescriptorSize);
            }
        }
        return hr;
    }
    HRESULT Renderer::createCommandAllocator()//command allocator생성(command list메모리 할당)
    {
        HRESULT hr = S_OK;
        for (UINT i = 0; i < FRAME_COUNT; i++)
        {
            hr = m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator[i]));
            if (FAILED(hr))
            {
                return hr;
            }
        }
        
        return hr;
    }
    BOOL Renderer::isDeviceSupportRayTracing(IDXGIAdapter1* adapter) const
    {
        ComPtr<ID3D12Device> testDevice(nullptr);
        D3D12_FEATURE_DATA_D3D12_OPTIONS5 featureSupportData = {};

        return SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&testDevice)))
            && SUCCEEDED(testDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &featureSupportData, sizeof(featureSupportData)))
            && featureSupportData.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED;
    }
    HRESULT Renderer::createRaytracingInterfaces()
    {
        HRESULT hr = S_OK;
        hr = m_device->QueryInterface(IID_PPV_ARGS(&m_dxrDevice));
        if (FAILED(hr))
        {
            return hr;
        }
        hr = m_commandList->QueryInterface(IID_PPV_ARGS(&m_dxrCommandList));
        if (FAILED(hr))
        {
            return hr;
        }
        return hr;
    }
    HRESULT Renderer::createRaytracingRootSignature()
    {
        HRESULT hr = S_OK;
        hr = m_globalRootSignature.Initialize(m_device.Get());
        if (FAILED(hr))
        {
            return hr;
        }
        hr = m_localRootSignature.Initialize(m_device.Get());
        if (FAILED(hr))
        {
            return hr;
        }
        return hr;
    }
    HRESULT Renderer::createRaytracingPipelineStateObject()
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
        }

        
        CD3DX12_HIT_GROUP_SUBOBJECT* hitGroup = raytracingPipeline.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();//hit group 서브 오브젝트 생성
        
        hitGroup->SetClosestHitShaderImport(L"MyClosestHitShader");//히트그룹과 연결될 셰이더진입점
        hitGroup->SetHitGroupExport(L"MyHitGroup");//히트 그룹 수출
        hitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);//이 히트그룹은 삼각형

        
        CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT* shaderConfig = raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();//Shader Config서브오브젝트 생성
        UINT payloadSize = 4 * sizeof(float);   // float4 color
        UINT attributeSize = 2 * sizeof(float); // float2 barycentrics
        shaderConfig->Config(payloadSize, attributeSize);// payload, attribute사이즈 정의(ray tracomg 셰이더에서 인자로 사용됨)

        
        {//미리 만들어둔 local root signature로 서브오브젝트 만들어 파이프라인에 적용(MyRaygenShader에)
            CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT* localRootSignature = raytracingPipeline.CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
            localRootSignature->SetRootSignature(m_localRootSignature.GetRootSignature().Get());
            CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT* rootSignatureAssociation = raytracingPipeline.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
            rootSignatureAssociation->SetSubobjectToAssociate(*localRootSignature);
            rootSignatureAssociation->AddExport(L"MyHitGroup");//My Cloesest Hit 셰이더에서 사용하겠다
        }

        //모든 Shader에서 사용할 gloabl root signature서브 오브젝트 적용
        CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT* globalRootSignature = raytracingPipeline.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();// 글로벌 루트시그니처 서브오브젝트생성
        globalRootSignature->SetRootSignature(m_globalRootSignature.GetRootSignature().Get());
        //파이프라인 옵션
        CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT* pipelineConfig = raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();// 파이프라인 config 서브오브젝트 생성 
        UINT maxRecursionDepth = 1;//1번만 recursion 하겠다
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
    HRESULT Renderer::createUAVDescriptorHeap()
    {
        HRESULT hr = S_OK;
        D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {
            .Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,//CBV,SRV,UAV타입이다
            .NumDescriptors = 3u,
            .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,//shader에서 볼수있다
            .NodeMask = 0
        };
        hr = m_device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&m_descriptorHeap));//create
        if (FAILED(hr))
        {
            return hr;
        }
        m_uavHeapDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);//cpu별로 상이한 descriptor사이즈 가져오기
        return hr;
    }
    HRESULT Renderer::createAccelerationStructure()
    {
        HRESULT hr = S_OK;
        m_commandList->Reset(m_commandAllocator[m_frameIndex].Get(), nullptr);
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
        {//TLAS,BLAS만드는 command list실행
            hr = m_commandList->Close();
            if (FAILED(hr))
            {
                return hr;
            }
            ID3D12CommandList* commandLists[] = { m_commandList.Get() };
            m_commandQueue->ExecuteCommandLists(ARRAYSIZE(commandLists), commandLists);
        }
        {//command list 실행완료 대기
            hr = waitForGPU();
            if (FAILED(hr))
            {
                return hr;
            }
        }
        return hr;
    }
    HRESULT Renderer::createShaderTable()
    {
        HRESULT hr = S_OK;
        void* rayGenShaderIdentifier = nullptr;
        void* missShaderIdentifier = nullptr;
        void* hitGroupShaderIdentifier = nullptr;

        // Get shader identifiers.
        UINT shaderIdentifierSize;
        {
            ComPtr<ID3D12StateObjectProperties> stateObjectProperties(nullptr);
            hr = m_dxrStateObject.As(&stateObjectProperties);//m_dxrStateObject가 ID3D12StateObjectProperties의 기능을 사용하겠다
            rayGenShaderIdentifier = stateObjectProperties->GetShaderIdentifier(L"MyRaygenShader");
            missShaderIdentifier = stateObjectProperties->GetShaderIdentifier(L"MyMissShader");
            hitGroupShaderIdentifier = stateObjectProperties->GetShaderIdentifier(L"MyHitGroup");
            shaderIdentifierSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
        }

        // Ray gen shader table
        {
            UINT numShaderRecords = 1;
            UINT shaderRecordSize = shaderIdentifierSize;
            ShaderTable rayGenShaderTable{};
            hr = rayGenShaderTable.Initialize(m_device.Get(), numShaderRecords, shaderRecordSize);
            if (FAILED(hr))
            {
                return hr;
            }
            rayGenShaderTable.Push_back(ShaderRecord(rayGenShaderIdentifier, shaderIdentifierSize));
            m_rayGenShaderTable = rayGenShaderTable.GetResource();
        }

        // Miss shader table
        {
            UINT numShaderRecords = 1;
            UINT shaderRecordSize = shaderIdentifierSize;
            ShaderTable missShaderTable{};
            hr = missShaderTable.Initialize(m_device.Get(), numShaderRecords, shaderRecordSize);
            if (FAILED(hr))
            {
                return hr;
            }
            missShaderTable.Push_back(ShaderRecord(missShaderIdentifier, shaderIdentifierSize));
            m_missShaderTable = missShaderTable.GetResource();
        }

        // Hit group shader table
        {
            
            struct RootArguments {
                CubeConstantBuffer cb;
            } rootArguments;
            m_cubeCB.albedo = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
            rootArguments.cb = m_cubeCB;

            UINT numShaderRecords = 2;
            UINT shaderRecordSize = shaderIdentifierSize + sizeof(rootArguments);
            ShaderTable hitGroupShaderTable{};
            hr = hitGroupShaderTable.Initialize(m_device.Get(), numShaderRecords, shaderRecordSize);
            if (FAILED(hr))
            {
                return hr;
            }
            hitGroupShaderTable.Push_back(ShaderRecord(hitGroupShaderIdentifier, shaderIdentifierSize, &rootArguments, sizeof(rootArguments)));

            m_cubeCB.albedo = XMFLOAT4(1.0f, 0.f, 0.f, 1.f);
            rootArguments.cb = m_cubeCB;
            hitGroupShaderTable.Push_back(ShaderRecord(hitGroupShaderIdentifier, shaderIdentifierSize, &rootArguments, sizeof(rootArguments)));
            m_hitGroupShaderTable = hitGroupShaderTable.GetResource();
        }
        return hr;
    }
    HRESULT Renderer::createRaytacingOutputResource(_In_ HWND hWnd)
    {
        HRESULT hr = S_OK;
        UINT width = 0u;
        UINT height = 0u;
        getWindowWidthHeight(hWnd, &width, &height);
        //format은 Swap chain의 format과 같아야함!
        CD3DX12_RESOURCE_DESC uavDesc = CD3DX12_RESOURCE_DESC::Tex2D(
            DXGI_FORMAT_R8G8B8A8_UNORM, 
            width, 
            height,
            1, 1, 1, 0, 
            D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
        );
        CD3DX12_HEAP_PROPERTIES defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        hr = m_device->CreateCommittedResource(//결과물 UAV버퍼 생성
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
        m_device->CreateUnorderedAccessView(m_raytracingOutput.Get(), nullptr, &UAVDesc, uavDescriptorHandle);
        m_raytracingOutputResourceUAVGpuDescriptor = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_descriptorHeap->GetGPUDescriptorHandleForHeapStart(), m_raytracingOutputResourceUAVDescriptorHeapIndex, m_uavHeapDescriptorSize);
        return hr;
    }
    UINT Renderer::createBufferSRV(_In_ ID3D12Resource* buffer, _Out_ D3D12_CPU_DESCRIPTOR_HANDLE* cpuDescriptorHandle, _Out_ D3D12_GPU_DESCRIPTOR_HANDLE* gpuDescriptorHandle, _In_ UINT numElements, _In_ UINT elementSize)
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
        
        m_device->CreateShaderResourceView(buffer, &srvDesc, *cpuDescriptorHandle);
        *gpuDescriptorHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_descriptorHeap->GetGPUDescriptorHandleForHeapStart(), descriptorIndex, m_uavHeapDescriptorSize);
        return descriptorIndex;
    }

}