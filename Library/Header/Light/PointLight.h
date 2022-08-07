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
        PointLight(_In_ XMVECTOR position);
        PointLight(const PointLight& other) = delete;
        PointLight(PointLight&& other) = delete;
        PointLight& operator=(const PointLight& other) = delete;
        PointLight& operator=(PointLight&& other) = delete;
        ~PointLight() = default;
        HRESULT Initialize(_In_ ID3D12Device* pDevice);
        virtual void Update(_In_ FLOAT deltaTime) = 0;
        XMVECTOR GetPosition() const;
    protected:
        XMVECTOR m_position; 
        
    };
}