#include "pch.h"
#include "Random\RandomSampleCounter.h"

namespace library
{
	RandomSampleCounter::RandomSampleCounter()
		:
		m_sampleCounterConstantBuffer(nullptr),
		m_mappedData(nullptr),
        m_sampleCount(0u)
	{}
	ComPtr<ID3D12Resource>& RandomSampleCounter::GetConstantBuffer()
	{
		return m_sampleCounterConstantBuffer;
	}
	HRESULT RandomSampleCounter::Initialize(_In_ ID3D12Device* pDevice)
	{
		HRESULT hr = S_OK;
        const D3D12_HEAP_PROPERTIES uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

        size_t cbSize = 256;
        const D3D12_RESOURCE_DESC constantBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(cbSize);
        hr = pDevice->CreateCommittedResource(
            &uploadHeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &constantBufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_sampleCounterConstantBuffer)
        );
        if (FAILED(hr))
        {
            return hr;
        }
        CD3DX12_RANGE readRange(0, 0);  
        hr = m_sampleCounterConstantBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_mappedData));//m_mappedBuffer�� Constant Buffer�� ���� �ּ� ����
        if (FAILED(hr))
        {
            return hr;
        }
        updateGPUConstantBuffer();
        return hr;
	}
    void RandomSampleCounter::Update(_In_ FLOAT deltaTime,_In_ UINT renderType,_In_ BOOL bPastFrameMoved)
    {
        if (renderType == 1)
        {//path tracer ����϶��� ����
            if (bPastFrameMoved)
                m_sampleCount = 0;//�������� �����ϸ� ���� �ʱ�ȭ
            updateGPUConstantBuffer();
            m_sampleCount++;
        }
    }
    void RandomSampleCounter::updateGPUConstantBuffer()
    {
        RandomSampleCounterConstantBuffer cb{
            .sampleCount = m_sampleCount
        };
        memcpy(m_mappedData, &cb, sizeof(cb));
    }
}