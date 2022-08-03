#include "Scene\Scene.h"
namespace library
{
    Scene::Scene() :
        m_renderables()
    {}

    HRESULT Scene::Initialize(_In_ ID3D12Device* pDevice)
    {
        HRESULT hr = S_OK;
        for (UINT i = 0; i < m_renderables.size(); i++)
        {
            hr = m_renderables[i]->Initialize(pDevice);
            if (FAILED(hr))
            {
                return hr;
            }
        }
        return hr;
    }
    void Scene::AddRenderable(_In_ std::shared_ptr<Renderable>& pRenderable)
    {
        m_renderables.push_back(pRenderable);
    }
    std::vector<std::shared_ptr<Renderable>>& Scene::GetRenderables()
    {
        return m_renderables;
    }
}