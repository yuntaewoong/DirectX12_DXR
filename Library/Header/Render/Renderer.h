#pragma once

#include "Common/Common.h"
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
        void Render(_In_ FLOAT deltaTime);
        void SetRenderMode(_In_ ERenderMode renderMode);
	private:
        ERenderMode m_renderMode;
        static const UINT FRAME_COUNT = 2;
        struct Vertex
        {
            XMFLOAT3 position;
            XMFLOAT4 color;
        };
        ComPtr<IDXGIFactory4> m_dxgiFactory;
        ComPtr<IDXGISwapChain3> m_swapChain;
        ComPtr<ID3D12Device> m_device;
        ComPtr<ID3D12Resource> m_renderTargets[FRAME_COUNT];
        ComPtr<ID3D12CommandAllocator> m_commandAllocator;
        ComPtr<ID3D12CommandQueue> m_commandQueue;
        ComPtr<ID3D12RootSignature> m_rootSignature;
        ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
        ComPtr<ID3D12PipelineState> m_pipelineState;
        ComPtr<ID3D12GraphicsCommandList> m_commandList;

        ComPtr<ID3D12Resource> m_vertexBuffer;
        D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

        UINT m_rtvDescriptorSize;
        UINT m_frameIndex;
        HANDLE m_fenceEvent;
        ComPtr<ID3D12Fence> m_fence;
        UINT64 m_fenceValue;
    private:
        HRESULT initializePipeLine(_In_ HWND hWnd);
        HRESULT initializeAssets();
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

        HRESULT createRootSignature();
        HRESULT createPipelineState();
        HRESULT createVertexBuffer();
	};
}