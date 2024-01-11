#include "pch.h"
#include "Camera\Camera.h"

namespace library
{
    Camera::Camera(_In_ const XMVECTOR& position) :
        m_cameraConstantBuffer(),
        m_mappedData(nullptr),
        m_yaw(0.0f),
        m_pitch(0.0f),
        m_moveLeftRight(0.0f),
        m_moveBackForward(0.0f),
        m_moveUpDown(0.0f),
        m_travelSpeed(3.f),
        m_rotationSpeed(1.f),
        m_cameraForward(DEFAULT_FORWARD),
        m_cameraRight(DEFAULT_RIGHT),
        m_cameraUp(DEFAULT_UP),
        m_eye(position),
        m_at(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f)),
        m_up(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)),
        m_rotation(XMMATRIX()),
        m_view(XMMATRIX())
        
    {}
    ComPtr<ID3D12Resource>& Camera::GetConstantBuffer()
    {
        return m_cameraConstantBuffer;
    }

    void Camera::HandleInput(_In_ const DirectionsInput& directions, _In_ const MouseRelativeMovement& mouseRelativeMovement, _In_ FLOAT deltaTime)
    {
        if (directions.bUp)
            m_moveUpDown += deltaTime * m_travelSpeed;
        if (directions.bDown)
            m_moveUpDown -= deltaTime * m_travelSpeed;
        if (directions.bFront)
            m_moveBackForward += deltaTime * m_travelSpeed;
        if (directions.bBack)
            m_moveBackForward -= deltaTime * m_travelSpeed;
        if (directions.bRight)
            m_moveLeftRight += deltaTime * m_travelSpeed;
        if (directions.bLeft)
            m_moveLeftRight -= deltaTime * m_travelSpeed;
        if (directions.bRotateLeft)
            m_yaw -= deltaTime * m_rotationSpeed;
        if (directions.bRotateRight)
            m_yaw += deltaTime * m_rotationSpeed;
        
        //m_yaw += mouseRelativeMovement.X * deltaTime * m_rotationSpeed;
        //m_pitch += mouseRelativeMovement.Y * deltaTime * m_rotationSpeed;
        
        //m_pitch = std::clamp(m_pitch, -XM_PIDIV2 + 0.01f, XM_PIDIV2 - 0.01f);//pitch range setting, pitch value must be in range(PI/2 > pitch > -PI/2)
    }
    HRESULT Camera::Initialize(_In_ ID3D12Device* pDevice)
    {
        HRESULT hr = S_OK;
        const D3D12_HEAP_PROPERTIES uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

        size_t cbSize = 256;//Constant버퍼는 256의 배수여야함
        const D3D12_RESOURCE_DESC constantBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(cbSize);
        hr = pDevice->CreateCommittedResource(
            &uploadHeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &constantBufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_cameraConstantBuffer)
        );
        if (FAILED(hr))
        {
            return hr;
        }
        CD3DX12_RANGE readRange(0, 0);  
        hr = m_cameraConstantBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_mappedData));//m_mappedBuffer에 Constant Buffer에 대한 주소 저장
        if (FAILED(hr))
        {
            return hr;
        }
        //카메라 CB업데이트후 바인딩
        CameraConstantBuffer cbCamera{
            .projectionToWorld = XMMatrixTranspose(getInverseViewProjectionMatrix()),
            .cameraPosition = m_eye
        };
        memcpy(m_mappedData, &cbCamera, sizeof(cbCamera));
        return hr;
    }
    void Camera::Update(_In_ FLOAT deltaTime)
    {
        m_rotation = DirectX::XMMatrixRotationRollPitchYaw(m_pitch, m_yaw, 0);
        m_at = XMVector3TransformCoord(DEFAULT_FORWARD, m_rotation);
        m_at = XMVector3Normalize(m_at);

        XMMATRIX RotateYTempMatrix = XMMATRIX();
        RotateYTempMatrix = XMMatrixRotationY(m_yaw);

        m_cameraRight = XMVector3TransformCoord(DEFAULT_RIGHT, RotateYTempMatrix);
        m_cameraUp = XMVector3TransformCoord(m_cameraUp, RotateYTempMatrix);
        m_cameraForward = XMVector3TransformCoord(DEFAULT_FORWARD, RotateYTempMatrix);

        m_eye += m_moveLeftRight * m_cameraRight;
        m_eye += m_moveBackForward * m_cameraForward;
        m_eye += m_moveUpDown * m_cameraUp;

        m_moveLeftRight = 0.0f;
        m_moveBackForward = 0.0f;
        m_moveUpDown = 0.0f;

        m_at = m_eye + m_at;

        m_view = XMMatrixLookAtLH(m_eye, m_at, m_up);

        //카메라 CB업데이트후 바인딩
        CameraConstantBuffer cbCamera{
            .projectionToWorld = XMMatrixTranspose(getInverseViewProjectionMatrix()),
            .cameraPosition = m_eye
        };
        memcpy(m_mappedData, &cbCamera, sizeof(cbCamera));
    }
    XMMATRIX Camera::getInverseViewProjectionMatrix() const
    {
        XMMATRIX view = XMMatrixLookAtLH(m_eye, m_at, m_up);
        XMMATRIX proj = XMMatrixPerspectiveFovLH(XM_PIDIV4, 4.f/3.f, 1.0f, 125.0f);
        XMMATRIX viewProj = view * proj;
        return XMMatrixInverse(nullptr, viewProj);
    }
}