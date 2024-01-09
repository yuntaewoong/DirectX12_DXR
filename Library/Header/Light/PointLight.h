#pragma once

#include "Common/Common.h"

namespace library
{
    /*
        PointLight의 Base 클래스
    */
    class PointLight
    {
    public:
        PointLight(_In_ XMVECTOR position,_In_ FLOAT lumen);
        PointLight(const PointLight& other) = delete;
        PointLight(PointLight&& other) = delete;
        PointLight& operator=(const PointLight& other) = delete;
        PointLight& operator=(PointLight&& other) = delete;
        ~PointLight() = default;
        HRESULT Initialize(_In_ const ComPtr<ID3D12Device>& pDevice);
        virtual void Update(_In_ FLOAT deltaTime) = 0;
        XMVECTOR GetPosition() const;
        FLOAT GetLumen() const;
    protected:
        XMVECTOR m_position; 
        FLOAT m_lumen;
    };
}