#include "Render/Renderer.h"
#include "ShaderTable\ShaderTable.h"
#include "CompiledShaders\BasicVertexShader.hlsl.h"
#include "CompiledShaders\BasicPixelShader.hlsl.h"
#include "CompiledShaders\BasicRayTracing.hlsl.h"
namespace library
{
    Renderer::Renderer() :
        m_dxgiFactory(nullptr),
        m_swapChain(nullptr),
        m_device(nullptr),
        m_renderTargets{ nullptr,nullptr },
        m_commandAllocator(nullptr),
        m_commandQueue(nullptr),
        m_rtvHeap(nullptr),
        m_commandList(nullptr),
        m_vertexBuffer(nullptr),
        m_vertexBufferView(),
        m_indexBuffer(nullptr),
        m_indexBufferView(),
        m_dxrDevice(nullptr),
        m_dxrCommandList(nullptr),
        m_dxrStateObject(nullptr),
        m_raytracingGlobalRootSignature(nullptr),
        m_raytracingLocalRootSignature(nullptr),
        m_bottomLevelAccelerationStructure(nullptr),
        m_topLevelAccelerationStructure(nullptr),
        m_uavHeap(nullptr),
        m_descriptorsAllocated(0u),
        m_uavHeapDescriptorSize(0u),
        m_rayGenCB(),
        m_missShaderTable(nullptr),
        m_hitGroupShaderTable(nullptr),
        m_rayGenShaderTable(nullptr),
        m_raytracingOutput(nullptr),
        m_raytracingOutputResourceUAVGpuDescriptor(),
        m_raytracingOutputResourceUAVDescriptorHeapIndex(0u),
        m_rtvDescriptorSize(0u),
        m_frameIndex(0u),
        m_fenceEvent(),
        m_fence(nullptr),
        m_fenceValue(0u)
    {
        m_rayGenCB.viewport = { -1.0f, -1.0f, 1.0f, 1.0f };
        m_rayGenCB.stencil =
        {
            -1 + (0.1f / (9/16.f)), -1 + 0.1f,
             1 - (0.1f / (9 / 16.f)), 1.0f - 0.1f
        };
    }
	HRESULT Renderer::Initialize(_In_ HWND hWnd)
	{
        HRESULT hr = S_OK;
        hr = initializePipeLine(hWnd);
        if (FAILED(hr))
        {
            return hr;
        }
        return S_OK;
	}
    void Renderer::Render(_In_ FLOAT deltaTime)
    {
        populateCommandList();//command 기록
        m_commandList->Close();//command list작성 종료
        ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
        m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);//command queue에 담긴 command list들 실행명령(비동기)
        
        m_swapChain->Present(1, 0);//백버퍼 교체

        waitForPreviousFrame();//gpu작업완료 대기
    }
    HRESULT Renderer::initializePipeLine(_In_ HWND hWnd)
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
        hr = createVertexBuffer();
        if (FAILED(hr))
        {
            return hr;
        }
        hr = createIndexBuffer();
        if (FAILED(hr))
        {
            return hr;
        }
        {//commandList 만들기
            hr = m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList));
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
            hr = m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
            if (FAILED(hr))
            {
                return hr;
            }
            m_fenceValue = 1;

            m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
            if (m_fenceEvent == nullptr)
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
                if (FAILED(hr))
                {
                    return hr;
                }
            }
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
        hr = createRaytacingOutputResource(hWnd);//output UAV만들기
        if (FAILED(hr))
        {
            return hr;
        }
        hr = waitForPreviousFrame();
        if (FAILED(hr))
        {
            return hr;
        }
        return hr;
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
        hr = m_commandAllocator->Reset();
        if (FAILED(hr))
        {
            return hr;
        }
        hr = m_commandList->Reset(m_commandAllocator.Get(), nullptr);
        if (FAILED(hr))
        {
            return hr;
        }
        m_commandList->SetComputeRootSignature(m_raytracingGlobalRootSignature.Get());//compute shader의 루트 시그니처 바인딩  
        m_commandList->SetDescriptorHeaps(1, m_uavHeap.GetAddressOf());//command list에 cpu descriptor heap을 바인딩(이 CPU heap 매핑된 GPU주소들을 사용하겠다)
        m_commandList->SetComputeRootDescriptorTable(
            static_cast<UINT>(EGlobalRootSignatureSlot::OutputViewSlot),
            m_raytracingOutputResourceUAVGpuDescriptor
        );//아웃풋 UAV텍스쳐 바인딩
        m_commandList->SetComputeRootShaderResourceView(
            static_cast<UINT>(EGlobalRootSignatureSlot::AccelerationStructureSlot),
            m_topLevelAccelerationStructure->GetGPUVirtualAddress()
        );//TLAS 바인딩
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
                .StrideInBytes = m_hitGroupShaderTable->GetDesc().Width
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
    HRESULT Renderer::waitForPreviousFrame()
    {
        //현재 waitForPreviousFrame함수는 매우 비효율적인 코드(CPU가 GPU끝날때까지 바보가됨(block))
        //D3D12HelloFrameBuffering <= 이걸 보면 깨달음을 얻을거라는 마소직원의 조언..

        const UINT64 fence = m_fenceValue;
        HRESULT hr = S_OK;
        hr = m_commandQueue->Signal(m_fence.Get(), fence);//command queue 작업이 끝나면 fence값 업데이트 해주세요 라는 뜻
        if (FAILED(hr))
        {
            return hr;
        }
        m_fenceValue++;

        
        if (m_fence->GetCompletedValue() < fence)
        {
            hr = m_fence->SetEventOnCompletion(fence, m_fenceEvent);//m_fence 개체가 fence값으로 업데이트가 완료되면 m_fenceEvent이벤트 signal
            if (FAILED(hr))
            {
                return hr;
            }
            WaitForSingleObject(m_fenceEvent, INFINITE);//m_fenceEvent가 signal될때까지 본 스레드 blocking
        }

        m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();//gpu작업이 끝났으니 백버퍼 인덱스 교체
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
            .Flags = 0u                                     //각종 flag
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
        hr = m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator));
        if (FAILED(hr))
        {
            return hr;
        }
        return hr;
    }
    HRESULT Renderer::createVertexBuffer()
    {//Vertex Buffer 만들기, directx12는 vertex buffer를 D3D12Resource로 봄
        HRESULT hr = S_OK;
        Vertex triangleVertices[] = 
        {
            XMFLOAT3(0,-1.f,0.f),
            XMFLOAT3(-1.f,1.f,0.f),
            XMFLOAT3(1.f,1.f,0.f)
        };
        const UINT vertexBufferSize = sizeof(triangleVertices);

        //현재 heap type을 upload로 한 상태로 vertex buffer를 gpu메모리에 생성하는데, 이는 좋지 않은 방법
        //GPU가 접근할때마다 마샬링이 일어난다고 마소직원이 주석을 남김
        CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_UPLOAD);
        D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
        hr = m_device->CreateCommittedResource(//힙 크기 == 데이터 크기로 힙과, 자원 할당
            &heapProperties,                    //힙 타입
            D3D12_HEAP_FLAG_NONE,
            &resourceDesc,                      //자원 크기정보
            D3D12_RESOURCE_STATE_GENERIC_READ,  //접근 정보
            nullptr,
            IID_PPV_ARGS(&m_vertexBuffer)       //CPU메모리에서 접근가능한 ComPtr개체
        );

        UINT8* pVertexDataBegin = nullptr;    // gpu메모리에 mapping 될 cpu메모리(virtual memory로 운영체제 통해 접근하는듯)
        CD3DX12_RANGE readRange(0, 0);        // 0~0으로 설정시 CPU메모리로 gpu데이터 읽기 불허 가능, nullptr입력하면 gpu데이터 읽기 가능
        hr = m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin));//매핑
        if (FAILED(hr))
        {
            return hr;
        }
        memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));//gpu 메모리 전송
        m_vertexBuffer->Unmap(0, nullptr);//매핑 해제

        m_vertexBufferView = {
            .BufferLocation = m_vertexBuffer->GetGPUVirtualAddress(),   //gpu메모리에 대응하는 cpu virtual address겟
            .SizeInBytes = vertexBufferSize,                            //vertex버퍼 총 크기는?
            .StrideInBytes = sizeof(Vertex)                             //각 vertex는 어떻게 띄어 읽어야하는가?
        };
        return hr;
    }
    HRESULT Renderer::createIndexBuffer()
    {
        HRESULT hr = S_OK;
        Index indices[] = 
        {
            0,1,2
        };
        const UINT indexBufferSize = sizeof(indices);

        CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_UPLOAD);//heap type은 upload
        D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize);
        hr = m_device->CreateCommittedResource(//힙 크기 == 데이터 크기로 힙과, 자원 할당
            &heapProperties,                    //힙 타입
            D3D12_HEAP_FLAG_NONE,
            &resourceDesc,                      //자원 크기정보
            D3D12_RESOURCE_STATE_GENERIC_READ,  //접근 정보
            nullptr,
            IID_PPV_ARGS(&m_indexBuffer)       //CPU메모리에서 접근가능한 ComPtr개체
        );
        if (FAILED(hr))
        {
            return hr;
        }
        UINT8* pIndexDataBegin = nullptr;    // gpu메모리에 mapping 될 cpu메모리(virtual memory로 운영체제 통해 접근하는듯)
        CD3DX12_RANGE readRange(0, 0);        // 0~0으로 설정시 CPU메모리로 gpu데이터 읽기 불허 가능, nullptr입력하면 gpu데이터 읽기 가능
        hr = m_indexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pIndexDataBegin));//매핑
        if (FAILED(hr))
        {
            return hr;
        }
        memcpy(pIndexDataBegin, indices, sizeof(indices));//gpu 메모리 전송
        m_indexBuffer->Unmap(0, nullptr);//매핑 해제

        m_indexBufferView = {
            .BufferLocation = m_indexBuffer->GetGPUVirtualAddress(),   //gpu메모리에 대응하는 cpu virtual address겟
            .SizeInBytes = indexBufferSize,                            //index버퍼 총 크기는?
            .Format = DXGI_FORMAT_R16_UINT                  //format은?
        };
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
        
        //global 루트 시그니처: 모든 Shader에서 사용할 자원 정의
        {
            ComPtr<ID3DBlob> signature(nullptr);
            ComPtr<ID3DBlob> error(nullptr);
            CD3DX12_DESCRIPTOR_RANGE UAVDescriptor{};
            UAVDescriptor.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
            CD3DX12_ROOT_PARAMETER rootParameters[NUM_OF_GLOBAL_ROOT_SIGNATURE] = {};
            rootParameters[static_cast<int>(EGlobalRootSignatureSlot::OutputViewSlot)].InitAsDescriptorTable(1, &UAVDescriptor);//렌더타겟 텍스처는 이렇게
            rootParameters[static_cast<int>(EGlobalRootSignatureSlot::AccelerationStructureSlot)].InitAsShaderResourceView(0);//AS는 이렇게 초기화
            CD3DX12_ROOT_SIGNATURE_DESC globalRootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters);
            hr = D3D12SerializeRootSignature(&globalRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);// 루트 시그니처의 binary화
            if (FAILED(hr))
            {
                return hr;
            }
            hr = m_device->CreateRootSignature(1, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_raytracingGlobalRootSignature));//루트 시그니처 생성
            if (FAILED(hr))
            {
                return hr;
            }
        }
        //local 루트 시그니처: 특정 Shader에서 사용할 자원 정의(Shader Table에서 선택)
        {
            ComPtr<ID3DBlob> signature(nullptr);
            ComPtr<ID3DBlob> error(nullptr);
            CD3DX12_ROOT_PARAMETER rootParameters[NUM_OF_LOCAL_ROOT_SIGNATURE] = {};
            rootParameters[static_cast<int>(ELocalRootSignatureSlot::ViewportConstantSlot)].InitAsConstants(SizeOfInUint32(m_rayGenCB), 0, 0);//Constant버퍼는 이렇게
            CD3DX12_ROOT_SIGNATURE_DESC localRootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters);
            localRootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;//이건 로컬이야
            hr = D3D12SerializeRootSignature(&localRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);// 루트 시그니처의 binary화
            if (FAILED(hr))
            {
                return hr;
            }
            hr = m_device->CreateRootSignature(1, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_raytracingLocalRootSignature));//루트 시그니처 생성
            if (FAILED(hr))
            {
                return hr;
            }
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
            localRootSignature->SetRootSignature(m_raytracingLocalRootSignature.Get());// 로컬 루트 시그니처 적용
            CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT* rootSignatureAssociation = raytracingPipeline.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
            rootSignatureAssociation->SetSubobjectToAssociate(*localRootSignature);
            rootSignatureAssociation->AddExport(L"MyRaygenShader");//ray gen 셰이더에서 사용하겠다
        }

        //모든 Shader에서 사용할 gloabl root signature서브 오브젝트 적용
        CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT* globalRootSignature = raytracingPipeline.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();// 글로벌 루트시그니처 서브오브젝트생성
        globalRootSignature->SetRootSignature(m_raytracingGlobalRootSignature.Get());//바로 적용(글로벌이니 association작업이 필요없다)

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
            .NumDescriptors = 1u,
            .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,//shader에서 볼수있다
            .NodeMask = 0
        };
        hr = m_device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&m_uavHeap));//create
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
        m_commandList->Reset(m_commandAllocator.Get(), nullptr);

        D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc = {//ray tracing geometry정보 구조체 정의, BLAS의 아이템 형식이라 할 수 있음
            .Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES,// geometry는 삼각형
            .Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE,//geometry는 불투명
            .Triangles = {
                .Transform3x4 = 0,
                .IndexFormat = DXGI_FORMAT_R16_UINT,
                .VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT,
                .IndexCount = static_cast<UINT>(m_indexBuffer->GetDesc().Width) / sizeof(Index),
                .VertexCount = static_cast<UINT>(m_vertexBuffer->GetDesc().Width) / sizeof(Vertex),
                .IndexBuffer = m_indexBuffer->GetGPUVirtualAddress(),
                .VertexBuffer = {
                    .StartAddress =  m_vertexBuffer->GetGPUVirtualAddress(),
                    .StrideInBytes = sizeof(Vertex)
                }
            }
        };
        // Get required sizes for an acceleration structure.
        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS topLevelInputs = {
            .Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL,//TLAS
            .Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE,
            .NumDescs = 1,
            .DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY
        };
        //AS 빌드 옵션 정의

        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo = {};
        m_dxrDevice->GetRaytracingAccelerationStructurePrebuildInfo(&topLevelInputs, &topLevelPrebuildInfo);//toplevelinput을 넣어주면 toplevelprebuildinfo가 나옴
        if (topLevelPrebuildInfo.ResultDataMaxSizeInBytes <= 0)//0바이트보다 큰지 검사
        {
            return E_FAIL;
        }

        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO bottomLevelPrebuildInfo = {};
        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS bottomLevelInputs = topLevelInputs;
        bottomLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;//BLAS
        bottomLevelInputs.pGeometryDescs = &geometryDesc;//BLAS에만 geometry가 있음
        m_dxrDevice->GetRaytracingAccelerationStructurePrebuildInfo(&bottomLevelInputs, &bottomLevelPrebuildInfo);
        if (bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes <= 0)//0바이트보다 큰지 검사
        {
            return E_FAIL;
        }

        ComPtr<ID3D12Resource> scratchResource(nullptr);
        CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
        CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(
            max(topLevelPrebuildInfo.ScratchDataSizeInBytes, bottomLevelPrebuildInfo.ScratchDataSizeInBytes),
            D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
        );
        hr = m_device->CreateCommittedResource(
            &uploadHeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
            nullptr,
            IID_PPV_ARGS(&scratchResource)
        );
        if (FAILED(hr))
        {
            return hr;
        }
        //TLAS,BLAS의 scratch데이터중 큰 크기만큼 UAV버퍼 하나 만들기

        {//BLAS 버퍼 만들기
            CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
            bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(
                bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes,
                D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
            );
            D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;//resource state는 acceleration structure
            
            hr = m_device->CreateCommittedResource(
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
        {//TLAS 버퍼 만들기
            CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
            bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(
                topLevelPrebuildInfo.ResultDataMaxSizeInBytes,
                D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
            );
            D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;//resource state는 acceleration structure
            hr = m_device->CreateCommittedResource(
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
        ComPtr<ID3D12Resource> instanceDescs(nullptr);//transform과 해당하는 BLAS주소
        {
            //TLAS에 들어가는 Instance버퍼 1개 만들기
            D3D12_RAYTRACING_INSTANCE_DESC instanceDesc = {
                .Transform = {
                    {1,0,0,0},
                    {0,1,0,0},
                    {0,0,1,0},
                },
                .InstanceID = 24,
                .InstanceMask = 1,
                .InstanceContributionToHitGroupIndex = 0,
                .AccelerationStructure = m_bottomLevelAccelerationStructure->GetGPUVirtualAddress()
            };
            CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
            CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(instanceDesc));
            hr = m_device->CreateCommittedResource(
                &uploadHeapProperties,
                D3D12_HEAP_FLAG_NONE,
                &bufferDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&instanceDescs)
            );
            void* pMappedData = nullptr;
            instanceDescs->Map(0, nullptr, &pMappedData);
            memcpy(pMappedData, &instanceDesc, sizeof(instanceDesc));
            instanceDescs->Unmap(0, nullptr);
            topLevelInputs.InstanceDescs = instanceDescs->GetGPUVirtualAddress();//top level input에 instance주소 추가
        }
        // TLAS빌드옵션
        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc = {
            .DestAccelerationStructureData = m_topLevelAccelerationStructure->GetGPUVirtualAddress(),//TLAS버퍼 주소
            .Inputs = topLevelInputs,                                                                //인풋 
            .SourceAccelerationStructureData = 0,                                                    //?
            .ScratchAccelerationStructureData = scratchResource->GetGPUVirtualAddress()              //scratch버퍼 주소
        };
        // BLAS빌드옵션
        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottomLevelBuildDesc = {
            .DestAccelerationStructureData = m_bottomLevelAccelerationStructure->GetGPUVirtualAddress(),//BLAS버퍼 주소
            .Inputs = bottomLevelInputs,                                                                //인풋
            .SourceAccelerationStructureData = 0,                                                       //?
            .ScratchAccelerationStructureData = scratchResource->GetGPUVirtualAddress()                 //scratch버퍼 주소
        };
        m_dxrCommandList->BuildRaytracingAccelerationStructure(&bottomLevelBuildDesc, 0, nullptr);//BLAS만들기
        CD3DX12_RESOURCE_BARRIER tempUAV = CD3DX12_RESOURCE_BARRIER::UAV(m_bottomLevelAccelerationStructure.Get());
        m_commandList->ResourceBarrier(1, &tempUAV);// UAV는 쓸때마다 Barrier (쓰기->쓰기 여도 Unordered때문에 그럼)
        m_dxrCommandList->BuildRaytracingAccelerationStructure(&topLevelBuildDesc, 0, nullptr);//TLAS만들기

        {//TLAS,BLAS만드는 command list실행
            hr = m_commandList->Close();
            if (FAILED(hr))
            {
                return hr;
            }
            ID3D12CommandList* commandLists[] = { m_commandList.Get() };
            m_commandQueue->ExecuteCommandLists(ARRAYSIZE(commandLists), commandLists);
        }
        hr = waitForPreviousFrame();//command list 실행완료 대기
        if (FAILED(hr))
        {
            return hr;
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
            struct RootArguments {
                RayGenConstantBuffer cb;
            } rootArguments;
            rootArguments.cb = m_rayGenCB;

            UINT numShaderRecords = 1;
            UINT shaderRecordSize = shaderIdentifierSize + sizeof(rootArguments);
            ShaderTable rayGenShaderTable{};
            hr = rayGenShaderTable.Initialize(m_device.Get(), numShaderRecords, shaderRecordSize);
            if (FAILED(hr))
            {
                return hr;
            }
            rayGenShaderTable.Push_back(ShaderRecord(rayGenShaderIdentifier, shaderIdentifierSize, &rootArguments, sizeof(rootArguments)));
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
            UINT numShaderRecords = 1;
            UINT shaderRecordSize = shaderIdentifierSize;
            ShaderTable hitGroupShaderTable{};
            hr = hitGroupShaderTable.Initialize(m_device.Get(), numShaderRecords, shaderRecordSize);
            if (FAILED(hr))
            {
                return hr;
            }
            hitGroupShaderTable.Push_back(ShaderRecord(hitGroupShaderIdentifier, shaderIdentifierSize));
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
        D3D12_CPU_DESCRIPTOR_HANDLE descriptorHeapCpuBase = m_uavHeap->GetCPUDescriptorHandleForHeapStart();
        if (m_raytracingOutputResourceUAVDescriptorHeapIndex >= m_uavHeap->GetDesc().NumDescriptors)
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
        m_raytracingOutputResourceUAVGpuDescriptor = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_uavHeap->GetGPUDescriptorHandleForHeapStart(), m_raytracingOutputResourceUAVDescriptorHeapIndex, m_uavHeapDescriptorSize);
        return hr;
    }
}