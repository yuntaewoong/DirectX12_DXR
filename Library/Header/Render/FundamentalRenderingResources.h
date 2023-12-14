#pragma once

#include "Common\Common.h"
#include "DescriptorHeap\RTVDescriptorHeap.h"
#include "DescriptorHeap\CBVSRVUAVDescriptorHeap.h"
namespace library
{
	/*
		FundamentalRenderingResources Ŭ����
		D3D12 ���ø����̼��̶�� ������ �־�� �ϸ� ���Ŀ� �������� �ʰ� ����� �ڿ��� ����
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
		
		ComPtr<ID3D12Device>& GetDevice();
		ComPtr<ID3D12GraphicsCommandList>& GetCommandList();
		ComPtr<ID3D12Resource>& GetCurrentRenderTarget();
		ComPtr<ID3D12CommandQueue>& GetCommandQueue();
		CBVSRVUAVDescriptorHeap& GetCBVSRVUAVDescriptorHeap();
		RTVDescriptorHeap& GetRTVDescriptorHeap();
		CBVSRVUAVDescriptorHeap& GetImguiDescriptorHeap();
		UINT GetWidth() const;
		UINT GetHeight() const;
		UINT GetFrameIndex() const;
	private:
		HRESULT getHardwareAdapter(
			_In_ IDXGIFactory1* pFactory,
			_Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter,
			bool requestHighPerformanceAdapter = false
		);
		BOOL isDeviceSupportRayTracing(IDXGIAdapter1* adapter) const;
		void setWindowWidthHeight(_In_ HWND hWnd);
		HRESULT createDevice();
		HRESULT createCommandQueue();
		HRESULT createSwapChain(_In_ HWND hWnd);
		HRESULT createRenderTargetView();
		HRESULT createCommandAllocator();
		HRESULT createFence();
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
		RTVDescriptorHeap m_rtvDescriptorHeap;
		CBVSRVUAVDescriptorHeap m_cbvSrvUavDescriptorHeap;
		CBVSRVUAVDescriptorHeap m_ImguiDescriptorHeap;

		//�潺
		Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
		UINT64 m_fenceValues[FRAME_COUNT];
		Microsoft::WRL::Wrappers::Event m_fenceEvent;

		UINT m_width;//���� �ػ�
		UINT m_height;//���� �ػ�
	};
}