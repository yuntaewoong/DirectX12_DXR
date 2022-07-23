#include "Render/Renderer.h"
namespace library
{
    Renderer::Renderer() :
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
    void Renderer::Render()
    {
        // Record all the commands we need to render the scene into the command list.
        populateCommandList();

        // Execute the command list.
        ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
        m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

        // Present the frame.
        m_swapChain->Present(1, 0);

        waitForPreviousFrame();
    }
	HRESULT Renderer::initializePipeLine(_In_ HWND hWnd)
	{
        RECT rc;
        GetClientRect(hWnd,&rc);
        UINT rect_width = static_cast<UINT>(rc.right - rc.left);
        UINT rect_height = static_cast<UINT>(rc.bottom - rc.top);
        

        UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
        // Enable the debug layer (requires the Graphics Tools "optional feature").
        // NOTE: Enabling the debug layer after device creation will invalidate the active device.
        {
            ComPtr<ID3D12Debug> debugController;
            if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
            {
                debugController->EnableDebugLayer();

                // Enable additional debug layers.
                dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
            }
        }
#endif

        ComPtr<IDXGIFactory4> factory;
        HRESULT hr = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory));
        if (FAILED(hr))
        {
            return hr;
        }
        
        ComPtr<IDXGIAdapter1> hardwareAdapter;
        getHardwareAdapter(factory.Get(), &hardwareAdapter);

        hr = D3D12CreateDevice(
            hardwareAdapter.Get(),
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&m_device)
        );
        if (FAILED(hr))
        {
            return hr;
        }

        // Describe and create the command queue.
        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

        hr = m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue));
        if (FAILED(hr))
        {
            return hr;
        }
        // Describe and create the swap chain.
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.BufferCount = FRAME_COUNT;
        swapChainDesc.Width = rect_width;
        swapChainDesc.Height = rect_height;
        swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.SampleDesc.Count = 1;

        ComPtr<IDXGISwapChain1> swapChain;
        hr = factory->CreateSwapChainForHwnd(
            m_commandQueue.Get(),        // Swap chain needs the queue so that it can force a flush on it.
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

        // This sample does not support fullscreen transitions.
        hr = factory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);
        if (FAILED(hr))
        {
            return hr;
        }

        hr = swapChain.As(&m_swapChain);
        if (FAILED(hr))
        {
            return hr;
        }
        m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

        // Create descriptor heaps.
        {
            // Describe and create a render target view (RTV) descriptor heap.
            D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
            rtvHeapDesc.NumDescriptors = FRAME_COUNT;
            rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            hr = m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap));
            if (FAILED(hr))
            {
                return hr;
            }
            m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        }

        // Create frame resources.
        {
            CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
            
            // Create a RTV for each frame.
            for (UINT n = 0; n < FRAME_COUNT; n++)
            {
                hr = m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n]));
                if (FAILED(hr))
                {
                    return hr;
                }
                m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
                rtvHandle.Offset(1, m_rtvDescriptorSize);
            }
        }

        hr = m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator));
        if (FAILED(hr))
        {
            return hr;
        }
	}
	HRESULT Renderer::initializeAssets()
	{
        HRESULT hr = S_OK;
        // Create the command list.
        hr = m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList));
        if (FAILED(hr))
        {
            return hr;
        }

        // Command lists are created in the recording state, but there is nothing
        // to record yet. The main loop expects it to be closed, so close it now.
        hr = m_commandList->Close();
        if (FAILED(hr))
        {
            return hr;
        }

        // Create synchronization objects.
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
        return S_OK;
	}
    void Renderer::getHardwareAdapter(
        _In_ IDXGIFactory1* pFactory,
        _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter,
        bool requestHighPerformanceAdapter)
    {
        *ppAdapter = nullptr;

        ComPtr<IDXGIAdapter1> adapter;

        ComPtr<IDXGIFactory6> factory6;
        if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6))))
        {
            for (
                UINT adapterIndex = 0;
                SUCCEEDED(factory6->EnumAdapterByGpuPreference(
                    adapterIndex,
                    requestHighPerformanceAdapter == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED,
                    IID_PPV_ARGS(&adapter)));
                ++adapterIndex)
            {
                DXGI_ADAPTER_DESC1 desc;
                adapter->GetDesc1(&desc);

                if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                {
                    // Don't select the Basic Render Driver adapter.
                    // If you want a software adapter, pass in "/warp" on the command line.
                    continue;
                }

                // Check to see whether the adapter supports Direct3D 12, but don't create the
                // actual device yet.
                if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
                {
                    break;
                }
            }
        }
    }
    HRESULT Renderer::populateCommandList()
    {
        // Command list allocators can only be reset when the associated 
        // command lists have finished execution on the GPU; apps should use 
        // fences to determine GPU execution progress.
        HRESULT hr = S_OK;
        hr = m_commandAllocator->Reset();
        if (FAILED(hr))
        {
            return hr;
        }

        // However, when ExecuteCommandList() is called on a particular command 
        // list, that command list can then be reset at any time and must be before 
        // re-recording.
        hr = m_commandList->Reset(m_commandAllocator.Get(), m_pipelineState.Get());
        if (FAILED(hr))
        {
            return hr;
        }
        // Indicate that the back buffer will be used as a render target.
        const D3D12_RESOURCE_BARRIER resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
            m_renderTargets[m_frameIndex].Get(),
            D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET
        );
        m_commandList->ResourceBarrier(1, &resourceBarrier);

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);

        // Record commands.
        const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
        m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

        const D3D12_RESOURCE_BARRIER resourceBarrier1 = CD3DX12_RESOURCE_BARRIER::Transition(
            m_renderTargets[m_frameIndex].Get(),
            D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT
        );
        // Indicate that the back buffer will now be used to present.
        m_commandList->ResourceBarrier(1,
            &resourceBarrier1
        );

        hr = m_commandList->Close();
        if (FAILED(hr))
        {
            return hr;
        }
        return S_OK;
    }
    HRESULT Renderer::waitForPreviousFrame()
    {
        // WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
    // This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
    // sample illustrates how to use fences for efficient resource usage and to
    // maximize GPU utilization.

    // Signal and increment the fence value.
        const UINT64 fence = m_fenceValue;
        HRESULT hr = S_OK;
        hr = m_commandQueue->Signal(m_fence.Get(), fence);
        if (FAILED(hr))
        {
            return hr;
        }
        m_fenceValue++;

        // Wait until the previous frame is finished.
        if (m_fence->GetCompletedValue() < fence)
        {
            hr = m_fence->SetEventOnCompletion(fence, m_fenceEvent);
            if (FAILED(hr))
            {
                return hr;
            }
            WaitForSingleObject(m_fenceEvent, INFINITE);
        }

        m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
        return S_OK;
    }
}