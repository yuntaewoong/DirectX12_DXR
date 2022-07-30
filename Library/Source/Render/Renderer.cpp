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
        populateCommandList();//command ���
        m_commandList->Close();//command list�ۼ� ����
        ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
        m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);//command queue�� ��� command list�� ������(�񵿱�)
        
        m_swapChain->Present(1, 0);//����� ��ü

        waitForPreviousFrame();//gpu�۾��Ϸ� ���
    }
    HRESULT Renderer::initializePipeLine(_In_ HWND hWnd)
    {
        HRESULT hr = S_OK;
        UINT dxgiFactoryFlags = 0;
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
        {//commandList �����
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
        {//fence�����
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
        //���ϴ� RayTracing�� �ʱ�ȭ
        ComPtr<IDXGIAdapter1> tempAdapter(nullptr);
        ComPtr<IDXGIFactory1> tempFactory(nullptr);
        getHardwareAdapter(m_dxgiFactory.Get(), tempAdapter.GetAddressOf(), false);
        if (!isDeviceSupportRayTracing(tempAdapter.Get()))
        {
            return E_FAIL;//����Ʈ���̽� ���� X
        }
        hr = createRaytracingInterfaces();//device,commandList ray tracing���������� query
        if (FAILED(hr))
        {
            return hr;
        }
        hr = createRaytracingRootSignature();//ray tracing���� ����� global,local root signature����
        if (FAILED(hr))
        {
            return hr;
        }
        hr = createRaytracingPipelineStateObject();//ray tracing pipeline����
        if (FAILED(hr))
        {
            return hr;
        }
        hr = createUAVDescriptorHeap();//ray tracing��� �׸� UAV�ؽ�ó�� ���� descriptor heap�����
        if (FAILED(hr))
        {
            return hr;
        }
        hr = createAccelerationStructure();//ray tracing�� geometry�� TLAS,BLAS�ڷᱸ���� �� �����ϱ�
        if (FAILED(hr))
        {
            return hr;
        }
        hr = createShaderTable();// shader table�����
        if (FAILED(hr))
        {
            return hr;
        }
        hr = createRaytacingOutputResource(hWnd);//output UAV�����
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
       �ϵ���� ��� ��������
    */
    HRESULT Renderer::getHardwareAdapter(
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
    HRESULT Renderer::populateCommandList()
    {
        
        //command list allocatorƯ: gpu���� ������ Reset����(�潺�� ����ȭ�ض�)
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
        m_commandList->SetComputeRootSignature(m_raytracingGlobalRootSignature.Get());//compute shader�� ��Ʈ �ñ״�ó ���ε�  
        m_commandList->SetDescriptorHeaps(1, m_uavHeap.GetAddressOf());//command list�� cpu descriptor heap�� ���ε�(�� CPU heap ���ε� GPU�ּҵ��� ����ϰڴ�)
        m_commandList->SetComputeRootDescriptorTable(
            static_cast<UINT>(EGlobalRootSignatureSlot::OutputViewSlot),
            m_raytracingOutputResourceUAVGpuDescriptor
        );//�ƿ�ǲ UAV�ؽ��� ���ε�
        m_commandList->SetComputeRootShaderResourceView(
            static_cast<UINT>(EGlobalRootSignatureSlot::AccelerationStructureSlot),
            m_topLevelAccelerationStructure->GetGPUVirtualAddress()
        );//TLAS ���ε�
        D3D12_DISPATCH_RAYS_DESC dispatchDesc = {//RayTracing���������� desc
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
        m_dxrCommandList->SetPipelineState1(m_dxrStateObject.Get());//������ ���� ray tracing pipeline���ε�
        m_dxrCommandList->DispatchRays(&dispatchDesc);// ��� �ȼ��� ���� ray generation shader������
        
        D3D12_RESOURCE_BARRIER preCopyBarriers[2] = {};
        preCopyBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(
            m_renderTargets[m_frameIndex].Get(),
            D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_COPY_DEST
        );//Render Target�� Copy�������� ����
        preCopyBarriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(
            m_raytracingOutput.Get(),
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE
        );//UAV���� Copy
        m_commandList->ResourceBarrier(ARRAYSIZE(preCopyBarriers), preCopyBarriers);

        m_commandList->CopyResource(m_renderTargets[m_frameIndex].Get(), m_raytracingOutput.Get());

        D3D12_RESOURCE_BARRIER postCopyBarriers[2] = {};
        postCopyBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(
            m_renderTargets[m_frameIndex].Get(),
            D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PRESENT
        );//Render Target�� present�ܰ�� �����
        postCopyBarriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(
            m_raytracingOutput.Get(),
            D3D12_RESOURCE_STATE_COPY_SOURCE,
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS
        );//UAV�� �ٽ� UA�� �����
        m_commandList->ResourceBarrier(ARRAYSIZE(postCopyBarriers), postCopyBarriers);
        
        return S_OK;
    }
    HRESULT Renderer::waitForPreviousFrame()
    {
        //���� waitForPreviousFrame�Լ��� �ſ� ��ȿ������ �ڵ�(CPU�� GPU���������� �ٺ�����(block))
        //D3D12HelloFrameBuffering <= �̰� ���� �������� �����Ŷ�� ���������� ����..

        const UINT64 fence = m_fenceValue;
        HRESULT hr = S_OK;
        hr = m_commandQueue->Signal(m_fence.Get(), fence);//command queue �۾��� ������ fence�� ������Ʈ ���ּ��� ��� ��
        if (FAILED(hr))
        {
            return hr;
        }
        m_fenceValue++;

        
        if (m_fence->GetCompletedValue() < fence)
        {
            hr = m_fence->SetEventOnCompletion(fence, m_fenceEvent);//m_fence ��ü�� fence������ ������Ʈ�� �Ϸ�Ǹ� m_fenceEvent�̺�Ʈ signal
            if (FAILED(hr))
            {
                return hr;
            }
            WaitForSingleObject(m_fenceEvent, INFINITE);//m_fenceEvent�� signal�ɶ����� �� ������ blocking
        }

        m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();//gpu�۾��� �������� ����� �ε��� ��ü
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
    HRESULT Renderer::createDevice()//d3d device����
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
    HRESULT Renderer::createCommandQueue()//Ŀ�ǵ� ť ����(Ŀ�ǵ� ť�� Ŀ�ǵ� ����Ʈ���� ť��)
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
    HRESULT Renderer::createSwapChain(_In_ HWND hWnd)//����ü�� ����
    {
        HRESULT hr = S_OK;
        UINT width = 0u;
        UINT height = 0u;
        getWindowWidthHeight(hWnd, &width, &height);
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {
            .Width = width,                                 //����
            .Height = height,                               //����
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
            .Flags = 0u                                     //���� flag
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
    HRESULT Renderer::createRenderTargetView()
    {
        HRESULT hr = S_OK;
        {//RTV�� descriptor heap ����
            D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {
                .Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,     //����Ÿ�ٺ� descriptor heap
                .NumDescriptors = FRAME_COUNT,              //FRAME���� ��ŭ
                .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
                .NodeMask = 0u
            };
            hr = m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap));
            if (FAILED(hr))
            {
                return hr;
            }
            m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);//descriptor heap �� ��� ������(�ü������ ����)
        }
        {//RTV����
            CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());//ù��° descriptor��������(iterator�� ����)
            for (UINT n = 0; n < FRAME_COUNT; n++)//������ ������ŭ
            {
                hr = m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n]));//����ü�ο��� ���� ��������
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
    HRESULT Renderer::createCommandAllocator()//command allocator����(command list�޸� �Ҵ�)
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
    {//Vertex Buffer �����, directx12�� vertex buffer�� D3D12Resource�� ��
        HRESULT hr = S_OK;
        Vertex triangleVertices[] = 
        {
            XMFLOAT3(0,-1.f,0.f),
            XMFLOAT3(-1.f,1.f,0.f),
            XMFLOAT3(1.f,1.f,0.f)
        };
        const UINT vertexBufferSize = sizeof(triangleVertices);

        //���� heap type�� upload�� �� ���·� vertex buffer�� gpu�޸𸮿� �����ϴµ�, �̴� ���� ���� ���
        //GPU�� �����Ҷ����� �������� �Ͼ�ٰ� ���������� �ּ��� ����
        CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_UPLOAD);
        D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
        hr = m_device->CreateCommittedResource(//�� ũ�� == ������ ũ��� ����, �ڿ� �Ҵ�
            &heapProperties,                    //�� Ÿ��
            D3D12_HEAP_FLAG_NONE,
            &resourceDesc,                      //�ڿ� ũ������
            D3D12_RESOURCE_STATE_GENERIC_READ,  //���� ����
            nullptr,
            IID_PPV_ARGS(&m_vertexBuffer)       //CPU�޸𸮿��� ���ٰ����� ComPtr��ü
        );

        UINT8* pVertexDataBegin = nullptr;    // gpu�޸𸮿� mapping �� cpu�޸�(virtual memory�� �ü�� ���� �����ϴµ�)
        CD3DX12_RANGE readRange(0, 0);        // 0~0���� ������ CPU�޸𸮷� gpu������ �б� ���� ����, nullptr�Է��ϸ� gpu������ �б� ����
        hr = m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin));//����
        if (FAILED(hr))
        {
            return hr;
        }
        memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));//gpu �޸� ����
        m_vertexBuffer->Unmap(0, nullptr);//���� ����

        m_vertexBufferView = {
            .BufferLocation = m_vertexBuffer->GetGPUVirtualAddress(),   //gpu�޸𸮿� �����ϴ� cpu virtual address��
            .SizeInBytes = vertexBufferSize,                            //vertex���� �� ũ���?
            .StrideInBytes = sizeof(Vertex)                             //�� vertex�� ��� ��� �о���ϴ°�?
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

        CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_UPLOAD);//heap type�� upload
        D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize);
        hr = m_device->CreateCommittedResource(//�� ũ�� == ������ ũ��� ����, �ڿ� �Ҵ�
            &heapProperties,                    //�� Ÿ��
            D3D12_HEAP_FLAG_NONE,
            &resourceDesc,                      //�ڿ� ũ������
            D3D12_RESOURCE_STATE_GENERIC_READ,  //���� ����
            nullptr,
            IID_PPV_ARGS(&m_indexBuffer)       //CPU�޸𸮿��� ���ٰ����� ComPtr��ü
        );
        if (FAILED(hr))
        {
            return hr;
        }
        UINT8* pIndexDataBegin = nullptr;    // gpu�޸𸮿� mapping �� cpu�޸�(virtual memory�� �ü�� ���� �����ϴµ�)
        CD3DX12_RANGE readRange(0, 0);        // 0~0���� ������ CPU�޸𸮷� gpu������ �б� ���� ����, nullptr�Է��ϸ� gpu������ �б� ����
        hr = m_indexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pIndexDataBegin));//����
        if (FAILED(hr))
        {
            return hr;
        }
        memcpy(pIndexDataBegin, indices, sizeof(indices));//gpu �޸� ����
        m_indexBuffer->Unmap(0, nullptr);//���� ����

        m_indexBufferView = {
            .BufferLocation = m_indexBuffer->GetGPUVirtualAddress(),   //gpu�޸𸮿� �����ϴ� cpu virtual address��
            .SizeInBytes = indexBufferSize,                            //index���� �� ũ���?
            .Format = DXGI_FORMAT_R16_UINT                  //format��?
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
        
        //global ��Ʈ �ñ״�ó: ��� Shader���� ����� �ڿ� ����
        {
            ComPtr<ID3DBlob> signature(nullptr);
            ComPtr<ID3DBlob> error(nullptr);
            CD3DX12_DESCRIPTOR_RANGE UAVDescriptor{};
            UAVDescriptor.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
            CD3DX12_ROOT_PARAMETER rootParameters[NUM_OF_GLOBAL_ROOT_SIGNATURE] = {};
            rootParameters[static_cast<int>(EGlobalRootSignatureSlot::OutputViewSlot)].InitAsDescriptorTable(1, &UAVDescriptor);//����Ÿ�� �ؽ�ó�� �̷���
            rootParameters[static_cast<int>(EGlobalRootSignatureSlot::AccelerationStructureSlot)].InitAsShaderResourceView(0);//AS�� �̷��� �ʱ�ȭ
            CD3DX12_ROOT_SIGNATURE_DESC globalRootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters);
            hr = D3D12SerializeRootSignature(&globalRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);// ��Ʈ �ñ״�ó�� binaryȭ
            if (FAILED(hr))
            {
                return hr;
            }
            hr = m_device->CreateRootSignature(1, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_raytracingGlobalRootSignature));//��Ʈ �ñ״�ó ����
            if (FAILED(hr))
            {
                return hr;
            }
        }
        //local ��Ʈ �ñ״�ó: Ư�� Shader���� ����� �ڿ� ����(Shader Table���� ����)
        {
            ComPtr<ID3DBlob> signature(nullptr);
            ComPtr<ID3DBlob> error(nullptr);
            CD3DX12_ROOT_PARAMETER rootParameters[NUM_OF_LOCAL_ROOT_SIGNATURE] = {};
            rootParameters[static_cast<int>(ELocalRootSignatureSlot::ViewportConstantSlot)].InitAsConstants(SizeOfInUint32(m_rayGenCB), 0, 0);//Constant���۴� �̷���
            CD3DX12_ROOT_SIGNATURE_DESC localRootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters);
            localRootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;//�̰� �����̾�
            hr = D3D12SerializeRootSignature(&localRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);// ��Ʈ �ñ״�ó�� binaryȭ
            if (FAILED(hr))
            {
                return hr;
            }
            hr = m_device->CreateRootSignature(1, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_raytracingLocalRootSignature));//��Ʈ �ñ״�ó ����
            if (FAILED(hr))
            {
                return hr;
            }
        }
        return hr;
    }
    HRESULT Renderer::createRaytracingPipelineStateObject()
    {
        CD3DX12_STATE_OBJECT_DESC raytracingPipeline{ D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE };//���� ���� state object�� ����Ʈ���̽� ����������


        //DXIL subobject
        CD3DX12_DXIL_LIBRARY_SUBOBJECT* lib = raytracingPipeline.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();//���̴��� wrapping�ϴ� DXIL���̺귯�� ���������Ʈ ����
        D3D12_SHADER_BYTECODE libdxil = CD3DX12_SHADER_BYTECODE(static_cast<const void*>(g_pBasicRayTracing), ARRAYSIZE(g_pBasicRayTracing));//���� Ÿ�� ������ ���̴� ����Ʈ�ڵ� ��������
        lib->SetDXILLibrary(&libdxil);//DXIL-Raytacing Shader ����
        {
            lib->DefineExport(L"MyRaygenShader");//ray generation shader������ ����
            lib->DefineExport(L"MyClosestHitShader");//closest hit shader������ ����
            lib->DefineExport(L"MyMissShader");//miss shader shader������ ����
        }

        
        CD3DX12_HIT_GROUP_SUBOBJECT* hitGroup = raytracingPipeline.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();//hit group ���� ������Ʈ ����
        
        hitGroup->SetClosestHitShaderImport(L"MyClosestHitShader");//��Ʈ�׷�� ����� ���̴�������
        hitGroup->SetHitGroupExport(L"MyHitGroup");//��Ʈ �׷� ����
        hitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);//�� ��Ʈ�׷��� �ﰢ��

        
        CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT* shaderConfig = raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();//Shader Config���������Ʈ ����
        UINT payloadSize = 4 * sizeof(float);   // float4 color
        UINT attributeSize = 2 * sizeof(float); // float2 barycentrics
        shaderConfig->Config(payloadSize, attributeSize);// payload, attribute������ ����(ray tracomg ���̴����� ���ڷ� ����)

        
        {//�̸� ������ local root signature�� ���������Ʈ ����� ���������ο� ����(MyRaygenShader��)
            CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT* localRootSignature = raytracingPipeline.CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
            localRootSignature->SetRootSignature(m_raytracingLocalRootSignature.Get());// ���� ��Ʈ �ñ״�ó ����
            CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT* rootSignatureAssociation = raytracingPipeline.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
            rootSignatureAssociation->SetSubobjectToAssociate(*localRootSignature);
            rootSignatureAssociation->AddExport(L"MyRaygenShader");//ray gen ���̴����� ����ϰڴ�
        }

        //��� Shader���� ����� gloabl root signature���� ������Ʈ ����
        CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT* globalRootSignature = raytracingPipeline.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();// �۷ι� ��Ʈ�ñ״�ó ���������Ʈ����
        globalRootSignature->SetRootSignature(m_raytracingGlobalRootSignature.Get());//�ٷ� ����(�۷ι��̴� association�۾��� �ʿ����)

        //���������� �ɼ�
        CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT* pipelineConfig = raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();// ���������� config ���������Ʈ ���� 
        UINT maxRecursionDepth = 1;//1���� recursion �ϰڴ�
        pipelineConfig->Config(maxRecursionDepth);//����


        //��ᰡ���� ���� StateObejct����
        HRESULT hr = S_OK;
        hr = m_dxrDevice->CreateStateObject(raytracingPipeline, IID_PPV_ARGS(&m_dxrStateObject));//����Ʈ���̽� Ŀ���� ���������� ����
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
            .Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,//CBV,SRV,UAVŸ���̴�
            .NumDescriptors = 1u,
            .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,//shader���� �����ִ�
            .NodeMask = 0
        };
        hr = m_device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&m_uavHeap));//create
        if (FAILED(hr))
        {
            return hr;
        }
        m_uavHeapDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);//cpu���� ������ descriptor������ ��������
        return hr;
    }
    HRESULT Renderer::createAccelerationStructure()
    {
        HRESULT hr = S_OK;
        m_commandList->Reset(m_commandAllocator.Get(), nullptr);

        D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc = {//ray tracing geometry���� ����ü ����, BLAS�� ������ �����̶� �� �� ����
            .Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES,// geometry�� �ﰢ��
            .Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE,//geometry�� ������
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
        //AS ���� �ɼ� ����

        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo = {};
        m_dxrDevice->GetRaytracingAccelerationStructurePrebuildInfo(&topLevelInputs, &topLevelPrebuildInfo);//toplevelinput�� �־��ָ� toplevelprebuildinfo�� ����
        if (topLevelPrebuildInfo.ResultDataMaxSizeInBytes <= 0)//0����Ʈ���� ū�� �˻�
        {
            return E_FAIL;
        }

        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO bottomLevelPrebuildInfo = {};
        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS bottomLevelInputs = topLevelInputs;
        bottomLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;//BLAS
        bottomLevelInputs.pGeometryDescs = &geometryDesc;//BLAS���� geometry�� ����
        m_dxrDevice->GetRaytracingAccelerationStructurePrebuildInfo(&bottomLevelInputs, &bottomLevelPrebuildInfo);
        if (bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes <= 0)//0����Ʈ���� ū�� �˻�
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
        //TLAS,BLAS�� scratch�������� ū ũ�⸸ŭ UAV���� �ϳ� �����

        {//BLAS ���� �����
            CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
            bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(
                bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes,
                D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
            );
            D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;//resource state�� acceleration structure
            
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
        {//TLAS ���� �����
            CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
            bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(
                topLevelPrebuildInfo.ResultDataMaxSizeInBytes,
                D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
            );
            D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;//resource state�� acceleration structure
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
        ComPtr<ID3D12Resource> instanceDescs(nullptr);//transform�� �ش��ϴ� BLAS�ּ�
        {
            //TLAS�� ���� Instance���� 1�� �����
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
            topLevelInputs.InstanceDescs = instanceDescs->GetGPUVirtualAddress();//top level input�� instance�ּ� �߰�
        }
        // TLAS����ɼ�
        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc = {
            .DestAccelerationStructureData = m_topLevelAccelerationStructure->GetGPUVirtualAddress(),//TLAS���� �ּ�
            .Inputs = topLevelInputs,                                                                //��ǲ 
            .SourceAccelerationStructureData = 0,                                                    //?
            .ScratchAccelerationStructureData = scratchResource->GetGPUVirtualAddress()              //scratch���� �ּ�
        };
        // BLAS����ɼ�
        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottomLevelBuildDesc = {
            .DestAccelerationStructureData = m_bottomLevelAccelerationStructure->GetGPUVirtualAddress(),//BLAS���� �ּ�
            .Inputs = bottomLevelInputs,                                                                //��ǲ
            .SourceAccelerationStructureData = 0,                                                       //?
            .ScratchAccelerationStructureData = scratchResource->GetGPUVirtualAddress()                 //scratch���� �ּ�
        };
        m_dxrCommandList->BuildRaytracingAccelerationStructure(&bottomLevelBuildDesc, 0, nullptr);//BLAS�����
        CD3DX12_RESOURCE_BARRIER tempUAV = CD3DX12_RESOURCE_BARRIER::UAV(m_bottomLevelAccelerationStructure.Get());
        m_commandList->ResourceBarrier(1, &tempUAV);// UAV�� �������� Barrier (����->���� ���� Unordered������ �׷�)
        m_dxrCommandList->BuildRaytracingAccelerationStructure(&topLevelBuildDesc, 0, nullptr);//TLAS�����

        {//TLAS,BLAS����� command list����
            hr = m_commandList->Close();
            if (FAILED(hr))
            {
                return hr;
            }
            ID3D12CommandList* commandLists[] = { m_commandList.Get() };
            m_commandQueue->ExecuteCommandLists(ARRAYSIZE(commandLists), commandLists);
        }
        hr = waitForPreviousFrame();//command list ����Ϸ� ���
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
            hr = m_dxrStateObject.As(&stateObjectProperties);//m_dxrStateObject�� ID3D12StateObjectProperties�� ����� ����ϰڴ�
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
        //format�� Swap chain�� format�� ���ƾ���!
        CD3DX12_RESOURCE_DESC uavDesc = CD3DX12_RESOURCE_DESC::Tex2D(
            DXGI_FORMAT_R8G8B8A8_UNORM, 
            width, 
            height,
            1, 1, 1, 0, 
            D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
        );
        CD3DX12_HEAP_PROPERTIES defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        hr = m_device->CreateCommittedResource(//����� UAV���� ����
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