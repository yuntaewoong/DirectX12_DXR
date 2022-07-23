#include "Render/Renderer.h"
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
        m_pipelineState(nullptr),
        m_commandList(nullptr),
        m_rtvDescriptorSize(0u),
        m_frameIndex(0u),
        m_fenceEvent(),
        m_fence(nullptr),
        m_fenceValue(0u)
    {}
	HRESULT Renderer::Initialize(_In_ HWND hWnd)
	{
        HRESULT hr = S_OK;
        hr = initializePipeLine(hWnd);
        if (FAILED(hr))
        {
            return hr;
        }
        hr = initializeAssets();
        if (FAILED(hr))
        {
            return hr;
        }
        return S_OK;
	}
    void Renderer::Render(_In_ FLOAT deltaTime)
    {
        populateCommandList();//command 기록
        
        ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
        m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);//command queue에 담긴 command list들 실행명령
        
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
        return hr;
	}
	HRESULT Renderer::initializeAssets()
	{
        HRESULT hr = S_OK;
        {
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
        {
            hr = m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
            if (FAILED(hr))
            {
                return hr;
            }
            m_fenceValue = 1;

            // Create an event handle to use for frame synchronization.
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

        //command list 특: ExecuteCommandList실행하면 언제든지 Reset가능
        hr = m_commandList->Reset(m_commandAllocator.Get(), m_pipelineState.Get());
        if (FAILED(hr))
        {
            return hr;
        }
        // 백버퍼를 RT으로 쓸것이라고 전달(barrier)
        const D3D12_RESOURCE_BARRIER resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
            m_renderTargets[m_frameIndex].Get(),
            D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET
        );
        m_commandList->ResourceBarrier(1, &resourceBarrier);

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
        const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
        m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);//RTV를 언젠가 클리어 해달라고 commandList에 기록

        // 백버퍼를 Present용으로 쓸것이라고 전달(barrier)
        const D3D12_RESOURCE_BARRIER resourceBarrier1 = CD3DX12_RESOURCE_BARRIER::Transition(
            m_renderTargets[m_frameIndex].Get(),
            D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT
        );
        m_commandList->ResourceBarrier(1,&resourceBarrier1);

        hr = m_commandList->Close();//command list작성 종료
        if (FAILED(hr))
        {
            return hr;
        }
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
}