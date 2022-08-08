#include "Light\PointLight.h"

namespace library
{
    PointLight::PointLight(_In_ XMVECTOR position) :
        m_position(position)
    {}
    HRESULT PointLight::Initialize(_In_ ID3D12Device* pDevice)
    {
        return S_OK;
    }
    XMVECTOR PointLight::GetPosition() const
    {
        return m_position;
    }
}