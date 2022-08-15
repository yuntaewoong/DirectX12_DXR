#include "Render\FundamentalRenderingResources.h"

namespace library
{
	FundamentalRenderingResources::FundamentalRenderingResources() :
		m_frameIndex(0u),
		m_dxgiFactory(nullptr),
		m_swapChain(nullptr),
		m_device(nullptr),
		m_renderTargets(),
		m_commandAllocator(),
		m_commandQueue(nullptr),
		m_commandList(nullptr),
		m_rtvDescriptorHeap(FRAME_COUNT),
        m_cbvSrvUavDescriptorHeap(),
		m_fence(nullptr),
		m_fenceValues(),
		m_fenceEvent()
	{}
	HRESULT FundamentalRenderingResources::Initialize(_In_ HWND hWnd)
	{
        HRESULT hr = S_OK;
        UINT dxgiFactoryFlags = 0;
        setWindowWidthHeight(hWnd);//�ػ� �� ����
#if defined(_DEBUG)//����� �����
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
        hr = m_rtvDescriptorHeap.Initialize(m_device);
        if (FAILED(hr))
        {
            return hr;
        }
        hr = m_cbvSrvUavDescriptorHeap.Initialize(m_device);
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
        {//commandList �����
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
        hr = createFence();
        if (FAILED(hr))
        {
            return hr;
        }
        ComPtr<IDXGIAdapter1> tempAdapter(nullptr);
        ComPtr<IDXGIFactory1> tempFactory(nullptr);
        getHardwareAdapter(m_dxgiFactory.Get(), tempAdapter.GetAddressOf(), false);
        if (!isDeviceSupportRayTracing(tempAdapter.Get()))
        {
            return E_FAIL;//����Ʈ���̽� ���� X
        }
        return hr;
	}
    HRESULT FundamentalRenderingResources::WaitForGPU() noexcept
    {
        HRESULT hr = S_OK;
        if (m_commandQueue && m_fence && m_fenceEvent.IsValid())
        {
            UINT64 fenceValue = m_fenceValues[m_frameIndex];
            hr = m_commandQueue->Signal(m_fence.Get(), fenceValue);//command queue �۾��� ������ fence�� fenceValue�� ������Ʈ ���ּ��� ��� ��
            if (FAILED(hr))
            {
                return hr;
            }   // Wait until the Signal has been processed.
            hr = m_fence->SetEventOnCompletion(fenceValue, m_fenceEvent.Get());//m_fence ��ü�� fence������ ������Ʈ�� �Ϸ�Ǹ� m_fenceEvent�̺�Ʈ signal
            if (FAILED(hr))
            {
                return hr;
            }
            WaitForSingleObjectEx(m_fenceEvent.Get(), INFINITE, FALSE);//m_fenceEvent�� signal�ɶ����� �� ������ blocking
            m_fenceValues[m_frameIndex]++;
            return hr;
        }
        return E_FAIL;

    }
    HRESULT FundamentalRenderingResources::MoveToNextFrame()
    {
        HRESULT hr = S_OK;
        const UINT64 currentFenceValue = m_fenceValues[m_frameIndex];

        hr = m_commandQueue->Signal(m_fence.Get(), currentFenceValue);

        m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
        if (m_fence->GetCompletedValue() < m_fenceValues[m_frameIndex])
        {
            hr = m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent.Get());
            if (FAILED(hr))
            {
                return hr;
            }
            WaitForSingleObjectEx(m_fenceEvent.Get(), INFINITE, FALSE);
        }
        m_fenceValues[m_frameIndex] = currentFenceValue + 1;
        return hr;
    }
    HRESULT FundamentalRenderingResources::ResetCommandAllocator()
    {
        HRESULT hr = S_OK;
        hr = m_commandAllocator[m_frameIndex]->Reset();
        if (FAILED(hr))
        {
            return hr;
        }
        return hr;
    }
    HRESULT FundamentalRenderingResources::ExecuteCommandList()
    {
        HRESULT hr = S_OK;
        hr = m_commandList->Close();
        if (FAILED(hr))
        {
            return hr;
        }
        ID3D12CommandList* commandLists[] = { m_commandList.Get() };
        m_commandQueue->ExecuteCommandLists(ARRAYSIZE(commandLists), commandLists);
        return hr;
    }
    void FundamentalRenderingResources::PresentSwapChain()
    {
        m_swapChain->Present(1, 0);//����� ��ü
    }
    HRESULT FundamentalRenderingResources::ResetCommandList()
    {
        HRESULT hr = S_OK;
        hr = m_commandList->Reset(m_commandAllocator[m_frameIndex].Get(), nullptr);
        if (FAILED(hr))
        {
            return hr;
        }
        return hr;
    }
    ComPtr<ID3D12Device>& FundamentalRenderingResources::GetDevice()
    {
        return m_device;
    }
    ComPtr<ID3D12GraphicsCommandList>& FundamentalRenderingResources::GetCommandList()
    {
        return m_commandList;
    }
    ComPtr<ID3D12Resource>& FundamentalRenderingResources::GetCurrentRenderTarget()
    {
        return m_renderTargets[m_frameIndex];
    }
    ComPtr<ID3D12CommandQueue>& FundamentalRenderingResources::GetCommandQueue()
    {
        return m_commandQueue;
    }
    CBVSRVUAVDescriptorHeap& FundamentalRenderingResources::GetCBVSRVUAVDescriptorHeap()
    {
        return m_cbvSrvUavDescriptorHeap;
    }
    UINT FundamentalRenderingResources::GetWidth() const
    {
        return m_width;
    }
    UINT FundamentalRenderingResources::GetHeight() const
    {
        return m_height;
    }
    BOOL FundamentalRenderingResources::isDeviceSupportRayTracing(IDXGIAdapter1* adapter) const
    {
        ComPtr<ID3D12Device> testDevice(nullptr);
        D3D12_FEATURE_DATA_D3D12_OPTIONS5 featureSupportData = {};

        return SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&testDevice)))
            && SUCCEEDED(testDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &featureSupportData, sizeof(featureSupportData)))
            && featureSupportData.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED;
    }
    void FundamentalRenderingResources::setWindowWidthHeight(_In_ HWND hWnd)
    {
        RECT rc = {
            .left = 0u,
            .top = 0u,
            .right = 0u,
            .bottom = 0u
        };
        GetClientRect(hWnd, &rc);
        m_width = static_cast<UINT>(rc.right - rc.left);
        m_height = static_cast<UINT>(rc.bottom - rc.top);
    }
    HRESULT FundamentalRenderingResources::getHardwareAdapter(
        _In_ IDXGIFactory1* pFactory,
        _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter,
        bool requestHighPerformanceAdapter)
    {
        *ppAdapter = nullptr;
        ComPtr<IDXGIAdapter1> adapter(nullptr);
        ComPtr<IDXGIFactory6> factory6(nullptr);
        if (FAILED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6))))// dxgi factory6�� �����ϴ°�?
        {
            return E_FAIL;
        }
        for (//���� gpu adapter�� ��ȸ
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
            adapter->GetDesc1(&desc);//��� ���� ��ȸ
            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)//����Ʈ���� ����ʹ� ���x
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
    
    
    HRESULT FundamentalRenderingResources::createDevice()//d3d device����
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
    HRESULT FundamentalRenderingResources::createCommandQueue()//Ŀ�ǵ� ť ����(Ŀ�ǵ� ť�� Ŀ�ǵ� ����Ʈ���� ť��)
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
    HRESULT FundamentalRenderingResources::createSwapChain(_In_ HWND hWnd)//����ü�� ����
    {
        HRESULT hr = S_OK;
        
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {
            .Width = m_width,                                 //����
            .Height = m_height,                               //����
            .Format = DXGI_FORMAT_R8G8B8A8_UNORM,           //�� �ȼ��� ���� ����������
            .Stereo = false,                                //Ǯ ��ũ���ΰ�?
            .SampleDesc = {
                .Count = 1u,
                .Quality = 0u
            },
            .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT, //������� cpu ���� �ɼ�
            .BufferCount = FRAME_COUNT,                     //���� ����
            .Scaling = DXGI_SCALING_NONE,                   //�����ϸ� ��������
            .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,    //���ҹ��
            .AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED,       //alpha�� ��뿩��
            .Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING                              //���� flag
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
        hr = m_dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);//alt + enter�� ��üȭ���ϴ°� ����
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
    HRESULT FundamentalRenderingResources::createRenderTargetView()
    {
        HRESULT hr = S_OK;
        for (UINT n = 0; n < FRAME_COUNT; n++)//������ ������ŭ
        {
            hr = m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n]));//����ü�ο��� Render Target���� �� ���� ��������
            if (FAILED(hr))
            {
                return hr;
            }
            hr = m_rtvDescriptorHeap.CreateRTV(m_device, m_renderTargets[n]);
            if (FAILED(hr))
            {
                return hr;
            }
        }
        return hr;
    }
    HRESULT FundamentalRenderingResources::createCommandAllocator()//command allocator����(command list�޸� �Ҵ�)
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
    HRESULT FundamentalRenderingResources::createFence()
    {
        HRESULT hr = S_OK;
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
        return hr;
    }
}