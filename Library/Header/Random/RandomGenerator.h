#pragma once

#include "Common/Common.h"

namespace library
{
    class RandomGenerator
    {
    public:
        RandomGenerator();
        RandomGenerator(const RandomGenerator& other) = delete;
        RandomGenerator(RandomGenerator&& other) = delete;
        RandomGenerator& operator=(const RandomGenerator& other) = delete;
        RandomGenerator& operator=(RandomGenerator&& other) = delete;
        virtual ~RandomGenerator() = default;
        ComPtr<ID3D12Resource>& GetConstantBuffer();
        HRESULT Initialize(_In_ ID3D12Device* pDevice);
        void Update(_In_ FLOAT deltaTime);
    private:
        ComPtr<ID3D12Resource> m_randomConstantBuffer;
    };
}