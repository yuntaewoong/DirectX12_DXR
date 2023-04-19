#include "pch.h"
#include "Scene\Scene.h"
namespace library
{
    Scene::Scene() :
        m_renderables(),
        m_lights(),
        m_materials(),
        m_pointLightsConstantBuffer(nullptr),
        m_pointLightMappedData(nullptr)
    {}

    HRESULT Scene::Initialize(
        _In_ const ComPtr<ID3D12Device>& pDevice,
        _In_ const ComPtr<ID3D12CommandQueue>& pCommandQueue,
        _In_ CBVSRVUAVDescriptorHeap& cbvSrvUavDescriptorHeap
    )
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
        for (UINT i = 0; i < m_models.size(); i++)
        {
            hr = m_models[i]->Initialize(pDevice);
            if (FAILED(hr))
            {
                return hr;
            }
        }
        for (UINT i = 0; i < m_lights.size(); i++)
        {
            hr = m_lights[i]->Initialize(pDevice);
            if (FAILED(hr))
            {
                return hr;
            }
        }
        for (UINT i = 0; i < m_materials.size(); i++)
        {
            hr = m_materials[i]->Initialize(pDevice, pCommandQueue, cbvSrvUavDescriptorHeap);
            if (FAILED(hr))
            {
                return hr;
            }
        }
        hr = createLightConstantBuffer(pDevice);
        if (FAILED(hr))
        {
            return hr;
        }
        return hr;
    }
    void Scene::AddRenderable(_In_ const std::shared_ptr<Renderable>& pRenderable)
    {
        m_renderables.push_back(pRenderable);
    }
    void Scene::AddModel(const std::shared_ptr<Model>& pModel)
    {
        m_models.push_back(pModel);
    }
    void Scene::AddMaterial(_In_ const std::shared_ptr<Material>& pMaterial)
    {
        m_materials.push_back(pMaterial);
    }
    const std::vector<std::shared_ptr<Renderable>>& Scene::GetRenderables() const
    {
        return m_renderables;
    }
    void Scene::AddLight(_In_ const std::shared_ptr<PointLight>& pLight)
    {
        m_lights.push_back(pLight);
    }
    void Scene::Update(_In_ FLOAT deltaTime)
    {
        for (UINT i = 0; i < m_renderables.size(); i++)
        {
            m_renderables[i]->Update(deltaTime);
        }
        for (UINT i = 0; i < m_lights.size(); i++)
        {
            m_lights[i]->Update(deltaTime);
        }
        PointLightConstantBuffer cb = {};
        for (UINT i = 0; i < m_lights.size(); i++)
        {
            cb.position[i] = m_lights[i]->GetPosition();
        }
        memcpy(m_pointLightMappedData, &cb, sizeof(cb));
    }
    ComPtr<ID3D12Resource>& Scene::GetPointLightsConstantBuffer()
    {
        return m_pointLightsConstantBuffer;
    }
    HRESULT Scene::createLightConstantBuffer(_In_ const ComPtr<ID3D12Device>& pDevice)
    {//light들에 대한 constant buffer만들기
        HRESULT hr = S_OK;
        const D3D12_HEAP_PROPERTIES uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

        size_t cbSize = 256;//왜인지는 모르겠지만 Constant버퍼는 256의 배수여야함
        const D3D12_RESOURCE_DESC constantBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(cbSize);

        hr = pDevice->CreateCommittedResource(
            &uploadHeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &constantBufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_pointLightsConstantBuffer)
        );
        if (FAILED(hr))
        {
            return hr;
        }
        CD3DX12_RANGE readRange(0, 0);
        hr = m_pointLightsConstantBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_pointLightMappedData));
        if (FAILED(hr))
        {
            return hr;
        }
        updateLightConstantBuffer();
        return hr;
    }
    void Scene::updateLightConstantBuffer()
    {
        PointLightConstantBuffer cb = {};
        for (UINT i = 0; i < m_lights.size(); i++)
        {
            cb.position[i] = m_lights[i]->GetPosition();
        }
        memcpy(m_pointLightMappedData, &cb, sizeof(cb));
    }
}