#pragma once

#include "Common/Common.h"

namespace library
{
    class Camera
    {
    public:
        Camera() = delete;
        Camera(_In_ const XMVECTOR& position,_In_ FLOAT aspectRatio);
        Camera(const Camera& other) = delete;
        Camera(Camera&& other) = delete;
        Camera& operator=(const Camera& other) = delete;
        Camera& operator=(Camera&& other) = delete;
        virtual ~Camera() = default;

        
        ComPtr<ID3D12Resource>& GetConstantBuffer();

        void HandleInput(_In_ const DirectionsInput& directions, _In_ const MouseRelativeMovement& mouseRelativeMovement, _In_ FLOAT deltaTime);
        BOOL IsPastFrameMoved() const;
        HRESULT Initialize(_In_ ID3D12Device* pDevice);
        void Update(_In_ FLOAT deltaTime);
    private:
        static constexpr const XMVECTORF32 DEFAULT_FORWARD = { 0.0f, 0.0f, 1.0f, 0.0f };
        static constexpr const XMVECTORF32 DEFAULT_RIGHT = { 1.0f, 0.0f, 0.0f, 0.0f };
        static constexpr const XMVECTORF32 DEFAULT_UP = { 0.0f, 1.0f, 0.0f, 0.0f };

        ComPtr<ID3D12Resource> m_cameraConstantBuffer;
        void* m_mappedData;

        FLOAT m_aspectRatio;
        FLOAT m_yaw;
        FLOAT m_pitch;

        FLOAT m_moveLeftRight;
        FLOAT m_moveBackForward;
        FLOAT m_moveUpDown;

        FLOAT m_travelSpeed;
        FLOAT m_rotationSpeed;

        XMVECTOR m_cameraForward;
        XMVECTOR m_cameraRight;
        XMVECTOR m_cameraUp;

        XMVECTOR m_eye;
        XMVECTOR m_at;
        XMVECTOR m_up;

        XMMATRIX m_rotation;
        XMMATRIX m_view;

        BOOL m_bPastFrameMoved;
    private:
        XMMATRIX getInverseViewProjectionMatrix() const;
    };
}