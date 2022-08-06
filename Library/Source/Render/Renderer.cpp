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
        m_dxrDevice(nullptr),
        m_dxrCommandList(nullptr),
        m_dxrStateObject(nullptr),
        m_raytracingGlobalRootSignature(nullptr),
        m_raytracingLocalRootSignature(nullptr),
        m_accelerationStructure(std::make_unique<AccelerationStructure>()),
        m_uavHeap(nullptr),
        m_descriptorsAllocated(0u),
        m_uavHeapDescriptorSize(0u),
        m_sceneCB(),
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
        m_vertexBufferGpuDescriptorHandle(),
        m_eye(),
        m_at(),
        m_up(),
        m_perFrameConstants(nullptr),
        m_mappedConstantData(nullptr)
    {}
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
    void Renderer::SetMainScene(_In_ std::shared_ptr<Scene>& pScene)
    {
        m_scene = pScene;
    }
    void Renderer::Render()
    {
        populateCommandList();//command ���
        m_commandList->Close();//command list�ۼ� ����
        ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
        m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);//command queue�� ��� command list�� ������(�񵿱�)
        
        m_swapChain->Present(1,0);//����� ��ü
        moveToNextFrame();

        //waitForGPU();//gpu�۾��Ϸ� ���
    }
    void Renderer::Update(_In_ FLOAT deltaTime)
    {
        // Rotate the camera around Y axis.
        {
            float secondsToRotateAround = 24.0f;
            float angleToRotateBy = 360.0f * (deltaTime / secondsToRotateAround);
            XMMATRIX rotate = XMMatrixRotationY(XMConvertToRadians(angleToRotateBy));
            m_eye = XMVector3Transform(m_eye, rotate);
            m_up = XMVector3Transform(m_up, rotate);
            m_at = XMVector3Transform(m_at, rotate);
            updateCameraMatrix();
        }

        // Rotate the second light around Y axis.
        {
            UINT prevFrameIndex = (m_frameIndex + FRAME_COUNT + 1) % FRAME_COUNT;
            float secondsToRotateAround = 8.0f;
            float angleToRotateBy = -360.0f * (deltaTime / secondsToRotateAround);
            XMMATRIX rotate = XMMatrixRotationY(XMConvertToRadians(angleToRotateBy));
            const XMVECTOR& prevLightPosition = m_sceneCB[prevFrameIndex].lightPosition;
            m_sceneCB[m_frameIndex].lightPosition = XMVector3Transform(prevLightPosition, rotate);
        }
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
        {//fence�����
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

        initializeScene();
        hr = m_scene->Initialize(m_device.Get());//Renderable���� �����ϴ� Scene�ʱ�ȭ
        if (FAILED(hr))
        {
            return hr;
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
        
        hr = createConstantBuffer();
        if (FAILED(hr))
        {
            return hr;
        }
        {//SRV����
            std::shared_ptr<Renderable>& tempRenderable = m_scene->GetRenderables()[0];//�׽�Ʈ�� �������� ���۷��� 1�� ��������
            createBufferSRV(tempRenderable->GetIndexBuffer().Get(), &m_indexBufferCpuDescriptorHandle, &m_indexBufferGpuDescriptorHandle, tempRenderable->GetNumIndices() / 2, 0);
            createBufferSRV(tempRenderable->GetVertexBuffer().Get(), &m_vertexBufferCpuDescriptorHandle, &m_vertexBufferGpuDescriptorHandle, tempRenderable->GetNumVertices(), 0);
        }
        

        hr = createRaytacingOutputResource(hWnd);//output UAV�����
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
        m_commandList->SetComputeRootSignature(m_raytracingGlobalRootSignature.Get());//compute shader�� ��Ʈ �ñ״�ó ���ε�  

        
        // Copy the updated scene constant buffer to GPU.
        memcpy(&m_mappedConstantData[m_frameIndex].constants, &m_sceneCB[m_frameIndex], sizeof(m_sceneCB[m_frameIndex]));
        auto cbGpuAddress = m_perFrameConstants->GetGPUVirtualAddress() + m_frameIndex * sizeof(m_mappedConstantData[0]);
        m_commandList->SetComputeRootConstantBufferView(
            static_cast<UINT>(EGlobalRootSignatureSlot::SceneConstantSlot),
            cbGpuAddress
        );

        m_commandList->SetDescriptorHeaps(1, m_uavHeap.GetAddressOf());//command list�� cpu descriptor heap�� ���ε�(�� CPU heap ���ε� GPU�ּҵ��� ����ϰڴ�)
        m_commandList->SetComputeRootDescriptorTable(
            static_cast<UINT>(EGlobalRootSignatureSlot::OutputViewSlot),
            m_raytracingOutputResourceUAVGpuDescriptor
        );//�ƿ�ǲ UAV�ؽ��� ���ε�

        m_commandList->SetComputeRootShaderResourceView(
            static_cast<UINT>(EGlobalRootSignatureSlot::AccelerationStructureSlot),
            m_accelerationStructure->GetTLAS()->GetTLASVirtualAddress()
        );//TLAS ���ε�

        m_commandList->SetComputeRootDescriptorTable(
            static_cast<UINT>(EGlobalRootSignatureSlot::VertexBuffersSlot),
            m_indexBufferGpuDescriptorHandle
        );//vertex,index���� ���ε�


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
                .StrideInBytes = m_hitGroupShaderTable->GetDesc().Width /2
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
        
        //global ��Ʈ �ñ״�ó: ��� Shader���� ����� �ڿ� ����
        {
            ComPtr<ID3DBlob> signature(nullptr);
            ComPtr<ID3DBlob> error(nullptr);
            CD3DX12_DESCRIPTOR_RANGE ranges[2] = {};
            ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);  // 0�� �������ʹ� Output UAV�ؽ�ó
            ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 1);  // 1������ 2���� �������ʹ� Vertex,Index����

            CD3DX12_ROOT_PARAMETER rootParameters[NUM_OF_GLOBAL_ROOT_SIGNATURE] = {};
            rootParameters[static_cast<int>(EGlobalRootSignatureSlot::OutputViewSlot)].InitAsDescriptorTable(1, &ranges[0]);//ranges[0]������ �ʱ�ȭ
            rootParameters[static_cast<int>(EGlobalRootSignatureSlot::AccelerationStructureSlot)].InitAsShaderResourceView(0);//t0�� �������ʹ� AS��
            rootParameters[static_cast<int>(EGlobalRootSignatureSlot::SceneConstantSlot)].InitAsConstantBufferView(0);//b0�� �������ʹ� Constant Buffer��
            rootParameters[static_cast<int>(EGlobalRootSignatureSlot::VertexBuffersSlot)].InitAsDescriptorTable(1, &ranges[1]);//ranges[1]������ �ʱ�ȭ
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
            rootParameters[static_cast<int>(ELocalRootSignatureSlot::CubeConstantSlot)].InitAsConstants(SizeOfInUint32(m_cubeCB), 1);//1�� �������� 
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
            rootSignatureAssociation->AddExport(L"MyHitGroup");//My Cloesest Hit ���̴����� ����ϰڴ�
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
            .NumDescriptors = 3u,
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
        m_commandList->Reset(m_commandAllocator[m_frameIndex].Get(), nullptr);
        {//AS�� �̷�� �Ǵ� Renderable�� ����
            std::vector<std::shared_ptr<Renderable>>& renderables = m_scene->GetRenderables();
            for (UINT i = 0; i < renderables.size(); i++)
            {
                m_accelerationStructure->AddRenderable(renderables[i]);
            }
        }
        {//TLAS,BLAS�ʱ�ȭ
            hr = m_accelerationStructure->Initialize(m_dxrDevice.Get(), m_dxrCommandList.Get());
            if (FAILED(hr))
            {
                return hr;
            }
        }
        {//TLAS,BLAS����� command list����
            hr = m_commandList->Close();
            if (FAILED(hr))
            {
                return hr;
            }
            ID3D12CommandList* commandLists[] = { m_commandList.Get() };
            m_commandQueue->ExecuteCommandLists(ARRAYSIZE(commandLists), commandLists);
        }
        {//command list ����Ϸ� ���
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
            hr = m_dxrStateObject.As(&stateObjectProperties);//m_dxrStateObject�� ID3D12StateObjectProperties�� ����� ����ϰڴ�
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
        auto descriptorHeapCpuBase = m_uavHeap->GetCPUDescriptorHandleForHeapStart();
        if (descriptorIndex >= m_uavHeap->GetDesc().NumDescriptors)
        {
            descriptorIndex = m_descriptorsAllocated++;
        }
        *cpuDescriptorHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(descriptorHeapCpuBase, descriptorIndex, m_rtvDescriptorSize);

        m_device->CreateShaderResourceView(buffer, &srvDesc, *cpuDescriptorHandle);
        *gpuDescriptorHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_uavHeap->GetGPUDescriptorHandleForHeapStart(), descriptorIndex, m_rtvDescriptorSize);
        return descriptorIndex;
    }
    void Renderer::initializeScene()
    {
        // Setup materials.
        {
            m_cubeCB.albedo = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
        }

        // Setup camera.
        {
            // Initialize the view and projection inverse matrices.
            m_eye = { 0.0f, 2.0f, -5.0f, 1.0f };
            m_at = { 0.0f, 0.0f, 0.0f, 1.0f };
            XMVECTOR right = { 1.0f, 0.0f, 0.0f, 0.0f };

            XMVECTOR direction = XMVector4Normalize(m_at - m_eye);
            m_up = XMVector3Normalize(XMVector3Cross(direction, right));

            // Rotate camera around Y axis.
            XMMATRIX rotate = XMMatrixRotationY(XMConvertToRadians(45.0f));
            m_eye = XMVector3Transform(m_eye, rotate);
            m_up = XMVector3Transform(m_up, rotate);

            updateCameraMatrix();
        }

        // Setup lights.
        {
            // Initialize the lighting parameters.
            XMFLOAT4 lightPosition;
            XMFLOAT4 lightAmbientColor;
            XMFLOAT4 lightDiffuseColor;

            lightPosition = XMFLOAT4(0.0f, 1.8f, -3.0f, 0.0f);
            m_sceneCB[m_frameIndex].lightPosition = XMLoadFloat4(&lightPosition);

            lightAmbientColor = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
            m_sceneCB[m_frameIndex].lightAmbientColor = XMLoadFloat4(&lightAmbientColor);

            lightDiffuseColor = XMFLOAT4(1.f, 1.0f, 1.f, 1.0f);
            m_sceneCB[m_frameIndex].lightDiffuseColor = XMLoadFloat4(&lightDiffuseColor);
        }

        // Apply the initial values to all frames' buffer instances.
        for (auto& sceneCB : m_sceneCB)
        {
            sceneCB = m_sceneCB[m_frameIndex];
        }
    }
    void Renderer::updateCameraMatrix()
    {
        m_sceneCB[m_frameIndex].cameraPosition = m_eye;
        float fovAngleY = 45.0f;
        XMMATRIX view = XMMatrixLookAtLH(m_eye, m_at, m_up);
        XMMATRIX proj = XMMatrixPerspectiveFovLH(XMConvertToRadians(fovAngleY), 1, 1.0f, 125.0f);
        XMMATRIX viewProj = view * proj;

        m_sceneCB[m_frameIndex].projectionToWorld = XMMatrixTranspose(XMMatrixInverse(nullptr, viewProj));
    }
    HRESULT Renderer::createConstantBuffer()
    {
        HRESULT hr = S_OK;
        // Create the constant buffer memory and map the CPU and GPU addresses
        const D3D12_HEAP_PROPERTIES uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

        // Allocate one constant buffer per frame, since it gets updated every frame.
        size_t cbSize = FRAME_COUNT * sizeof(AlignedSceneConstantBuffer);//�������� �𸣰����� Constant���۴� 256�� ���������
        const D3D12_RESOURCE_DESC constantBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(cbSize);

        hr = m_device->CreateCommittedResource(
            &uploadHeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &constantBufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_perFrameConstants)
        );
        if (FAILED(hr))
        {
            return hr;
        }
        // Map the constant buffer and cache its heap pointers.
        // We don't unmap this until the app closes. Keeping buffer mapped for the lifetime of the resource is okay.
        CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
        hr = m_perFrameConstants->Map(0, nullptr, reinterpret_cast<void**>(&m_mappedConstantData));
        if (FAILED(hr))
        {
            return hr;
        }
        return hr;
    }

}