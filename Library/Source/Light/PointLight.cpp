#include "pch.h"
#include "Light\PointLight.h"

namespace library
{
    PointLight::PointLight(_In_ XMVECTOR position,_In_ FLOAT lumen) :
        m_position(position),
        m_lumen(lumen)
    {}
    HRESULT PointLight::Initialize(_In_ const ComPtr<ID3D12Device>& pDevice)
    {
        return S_OK;
    }
    XMVECTOR PointLight::GetPosition() const
    {
        return m_position;
    }
    FLOAT PointLight::GetLumen() const
    {
        return m_lumen;
    }
}