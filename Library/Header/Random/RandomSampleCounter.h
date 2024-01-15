#pragma once

#include "Common/Common.h"
#include "Camera\Camera.h"

namespace library
{
    class RandomSampleCounter
    {
    public:
        RandomSampleCounter();
        RandomSampleCounter(const RandomSampleCounter& other) = delete;
        RandomSampleCounter(RandomSampleCounter&& other) = delete;
        RandomSampleCounter& operator=(const RandomSampleCounter& other) = delete;
        RandomSampleCounter& operator=(RandomSampleCounter&& other) = delete;
        virtual ~RandomSampleCounter() = default;
        ComPtr<ID3D12Resource>& GetConstantBuffer();
        HRESULT Initialize(_In_ ID3D12Device* pDevice);
        void Update(_In_ FLOAT deltaTime,_In_ UINT renderType,_In_ BOOL bPastFrameMoved);
        UINT GetCurrentSampleCount() const;
    private:
        void updateGPUConstantBuffer();
    private:
        ComPtr<ID3D12Resource> m_sampleCounterConstantBuffer;
        void* m_mappedData;
        UINT m_sampleCount;
    };
}