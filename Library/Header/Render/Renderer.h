#pragma once

#include "Common/Common.h"
#include "AccelerationStructure\AccelerationStructure.h"
#include "Scene\Scene.h"
#include "Camera\Camera.h"
#include "RootSignature\GlobalRootSignature.h"
#include "RootSignature\LocalRootSignature.h"
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
        void HandleInput(_In_ const DirectionsInput& directions, _In_ const MouseRelativeMovement& mouseRelativeMovement, _In_ FLOAT deltaTime);
        void SetMainScene(_In_ std::shared_ptr<Scene>& pScene);
        void Render();
        void Update(_In_ FLOAT deltaTime);
	private:
        static const UINT FRAME_COUNT = 3;
        std::shared_ptr<Scene> m_scene;

        ComPtr<IDXGIFactory4> m_dxgiFactory;
        ComPtr<IDXGISwapChain3> m_swapChain;
        ComPtr<ID3D12Device> m_device;
        ComPtr<ID3D12Resource> m_renderTargets[FRAME_COUNT];
        ComPtr<ID3D12CommandAllocator> m_commandAllocator[FRAME_COUNT];
        ComPtr<ID3D12CommandQueue> m_commandQueue;
        ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
        ComPtr<ID3D12GraphicsCommandList> m_commandList;
        std::unique_ptr<Camera> m_camera;

        // DXR���������� ����
        ComPtr<ID3D12Device5> m_dxrDevice;
        ComPtr<ID3D12GraphicsCommandList4> m_dxrCommandList;
        ComPtr<ID3D12StateObject> m_dxrStateObject;

        //ray tracing���� root signature
        GlobalRootSignature m_globalRootSignature;
        LocalRootSignature m_localRootSignature;
        
        //ray tracing BLAS,TLAS
        std::unique_ptr<AccelerationStructure> m_accelerationStructure;

        //Ray tracing Shader���� �����ϴ� UAV�� ���� descriptor heap
        ComPtr<ID3D12DescriptorHeap> m_descriptorHeap;
        UINT m_descriptorsAllocated;
        UINT m_uavHeapDescriptorSize;

        
        CubeConstantBuffer m_cubeCB;

        //ray tracing Shader Table�ڿ�
        ComPtr<ID3D12Resource> m_missShaderTable;
        ComPtr<ID3D12Resource> m_hitGroupShaderTable;
        ComPtr<ID3D12Resource> m_rayGenShaderTable;

        // Raytracing  ��� UAV�ڿ�
        ComPtr<ID3D12Resource> m_raytracingOutput;
        D3D12_GPU_DESCRIPTOR_HANDLE m_raytracingOutputResourceUAVGpuDescriptor;
        UINT m_raytracingOutputResourceUAVDescriptorHeapIndex;

        UINT m_rtvDescriptorSize;
        UINT m_frameIndex;

        //�潺
        Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
        UINT64 m_fenceValues[FRAME_COUNT];
        Microsoft::WRL::Wrappers::Event m_fenceEvent;

        D3D12_CPU_DESCRIPTOR_HANDLE m_indexBufferCpuDescriptorHandle;
        D3D12_GPU_DESCRIPTOR_HANDLE m_indexBufferGpuDescriptorHandle;
        D3D12_CPU_DESCRIPTOR_HANDLE m_vertexBufferCpuDescriptorHandle;
        D3D12_GPU_DESCRIPTOR_HANDLE m_vertexBufferGpuDescriptorHandle;
        
    private:
        HRESULT getHardwareAdapter(
            _In_ IDXGIFactory1* pFactory,
            _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter,
            bool requestHighPerformanceAdapter = false
        );
        HRESULT populateCommandList();
        void getWindowWidthHeight(_In_ HWND hWnd, _Out_ PUINT pWidth, _Out_ PUINT pHeight);
        HRESULT waitForGPU() noexcept;
        HRESULT moveToNextFrame();

        HRESULT createDevice();
        HRESULT createCommandQueue();
        HRESULT createSwapChain(_In_ HWND hWnd);
        HRESULT createRenderTargetView();
        HRESULT createCommandAllocator();

        //���� �Լ��� ray tacing �����Լ�
        BOOL isDeviceSupportRayTracing(IDXGIAdapter1* adapter) const;
        HRESULT createRaytracingInterfaces();
        HRESULT createRaytracingRootSignature();
        HRESULT createRaytracingPipelineStateObject();
        HRESULT createUAVDescriptorHeap();
        HRESULT createAccelerationStructure();
        HRESULT createShaderTable();
        HRESULT createRaytacingOutputResource(_In_ HWND hWnd);
        UINT createBufferSRV(_In_ ID3D12Resource* buffer,_Out_ D3D12_CPU_DESCRIPTOR_HANDLE* cpuDescriptorHandle, _Out_ D3D12_GPU_DESCRIPTOR_HANDLE* gpuDescriptorHandle, _In_ UINT numElements, _In_ UINT elementSize);
	};
}