#pragma once

#include "Common\Common.h"
namespace library
{
	/*
		FundamentalRenderingResources 클래스
		D3D12 어플리케이션이라면 마땅히 있어야 하며 이후에 수정하지 않고 사용할 자원들 관리
	*/
	class FundamentalRenderingResources final
	{
	public:
		FundamentalRenderingResources();
		FundamentalRenderingResources(const FundamentalRenderingResources& other) = delete;
		FundamentalRenderingResources(FundamentalRenderingResources&& other) = delete;
		FundamentalRenderingResources& operator=(const FundamentalRenderingResources& other) = delete;
		FundamentalRenderingResources& operator=(FundamentalRenderingResources&& other) = delete;
		~FundamentalRenderingResources() = default;

		HRESULT Initialize(_In_ HWND hWnd);
		HRESULT WaitForGPU() noexcept;
		HRESULT MoveToNextFrame();
		HRESULT ResetCommandAllocator();
		HRESULT ResetCommandList();
		HRESULT ExecuteCommandList();
		void PresentSwapChain();
		void GetWindowWidthHeight(_In_ HWND hWnd, _Out_ PUINT pWidth, _Out_ PUINT pHeight);
		ComPtr<ID3D12Device>& GetDevice();
		ComPtr<ID3D12GraphicsCommandList>& GetCommandList();
		ComPtr<ID3D12Resource>& GetCurrentRenderTarget();
	private:
		HRESULT getHardwareAdapter(
			_In_ IDXGIFactory1* pFactory,
			_Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter,
			bool requestHighPerformanceAdapter = false
		);
		BOOL isDeviceSupportRayTracing(IDXGIAdapter1* adapter) const;
		HRESULT createDevice();
		HRESULT createCommandQueue();
		HRESULT createSwapChain(_In_ HWND hWnd);
		HRESULT createRenderTargetView();
		HRESULT createCommandAllocator();
	private:
		static const UINT FRAME_COUNT = 3;
		UINT m_frameIndex;
		ComPtr<IDXGIFactory4> m_dxgiFactory;
		ComPtr<IDXGISwapChain3> m_swapChain;
		ComPtr<ID3D12Device> m_device;
		ComPtr<ID3D12Resource> m_renderTargets[FRAME_COUNT];
		ComPtr<ID3D12CommandAllocator> m_commandAllocator[FRAME_COUNT];
		ComPtr<ID3D12CommandQueue> m_commandQueue;
		ComPtr<ID3D12GraphicsCommandList> m_commandList;
		ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
		UINT m_rtvDescriptorSize;

		//펜스
		Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
		UINT64 m_fenceValues[FRAME_COUNT];
		Microsoft::WRL::Wrappers::Event m_fenceEvent;
	};
}