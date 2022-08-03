#pragma once

#include "Common/Common.h"
#include "AccelerationStructure\AccelerationStructure.h"
#include "Scene\Scene.h"
namespace library
{
	class Renderer
	{
	public:
		Renderer();
		Renderer(const Renderer& other) = delete;
		Renderer(Renderer&& other) = delete;
		Renderer& operator=(const Renderer& other) = delete;
		Renderer& operator=(Renderer&& other) = delete;
		~Renderer() = default;

        HRESULT Initialize(_In_ HWND hWnd);
        void SetMainScene(_In_ std::shared_ptr<Scene>& pScene);
        void Render(_In_ FLOAT deltaTime);
	private:
        static const UINT FRAME_COUNT = 2;
        std::shared_ptr<Scene> m_scene;

        ComPtr<IDXGIFactory4> m_dxgiFactory;
        ComPtr<IDXGISwapChain3> m_swapChain;
        ComPtr<ID3D12Device> m_device;
        ComPtr<ID3D12Resource> m_renderTargets[FRAME_COUNT];
        ComPtr<ID3D12CommandAllocator> m_commandAllocator;
        ComPtr<ID3D12CommandQueue> m_commandQueue;
        ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
        ComPtr<ID3D12GraphicsCommandList> m_commandList;

        ComPtr<ID3D12Resource> m_vertexBuffer;
        ComPtr<ID3D12Resource> m_indexBuffer;
        // DXR파이프라인 관련
        ComPtr<ID3D12Device5> m_dxrDevice;
        ComPtr<ID3D12GraphicsCommandList4> m_dxrCommandList;
        ComPtr<ID3D12StateObject> m_dxrStateObject;

        //ray tracing전용 root signature
        ComPtr<ID3D12RootSignature> m_raytracingGlobalRootSignature;
        ComPtr<ID3D12RootSignature> m_raytracingLocalRootSignature;
        
        //ray tracing BLAS,TLAS
        std::unique_ptr<AccelerationStructure> m_accelerationStructure;

        //Ray tracing Shader에서 접근하는 UAV에 대한 descriptor heap
        ComPtr<ID3D12DescriptorHeap> m_uavHeap;
        UINT m_descriptorsAllocated;
        UINT m_uavHeapDescriptorSize;

        //RayGeneration Shader Constant Buffer 구조체
        RayGenConstantBuffer m_rayGenCB;

        //ray tracing Shader Table자원
        ComPtr<ID3D12Resource> m_missShaderTable;
        ComPtr<ID3D12Resource> m_hitGroupShaderTable;
        ComPtr<ID3D12Resource> m_rayGenShaderTable;

        // Raytracing  결과 UAV자원
        ComPtr<ID3D12Resource> m_raytracingOutput;
        D3D12_GPU_DESCRIPTOR_HANDLE m_raytracingOutputResourceUAVGpuDescriptor;
        UINT m_raytracingOutputResourceUAVDescriptorHeapIndex;

        UINT m_rtvDescriptorSize;
        UINT m_frameIndex;
        HANDLE m_fenceEvent;
        ComPtr<ID3D12Fence> m_fence;
        UINT64 m_fenceValue;


    private:
        HRESULT initializePipeLine(_In_ HWND hWnd);
        HRESULT getHardwareAdapter(
            _In_ IDXGIFactory1* pFactory,
            _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter,
            bool requestHighPerformanceAdapter = false
        );
        HRESULT populateCommandList();
        HRESULT waitForPreviousFrame();
        void getWindowWidthHeight(_In_ HWND hWnd, _Out_ PUINT pWidth, _Out_ PUINT pHeight);
        

        HRESULT createDevice();
        HRESULT createCommandQueue();
        HRESULT createSwapChain(_In_ HWND hWnd);
        HRESULT createRenderTargetView();
        HRESULT createCommandAllocator();

        HRESULT createVertexBuffer();
        HRESULT createIndexBuffer();

        //이하 함수는 ray tacing 관련함수
        BOOL isDeviceSupportRayTracing(IDXGIAdapter1* adapter) const;
        HRESULT createRaytracingInterfaces();
        HRESULT createRaytracingRootSignature();
        HRESULT createRaytracingPipelineStateObject();
        HRESULT createUAVDescriptorHeap();
        HRESULT createAccelerationStructure();
        HRESULT createShaderTable();
        HRESULT createRaytacingOutputResource(_In_ HWND hWnd);
	};
}