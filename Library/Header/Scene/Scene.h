#pragma once

#include "Common/Common.h"
#include "Render\Renderable.h"
#include "Model\Model.h"
#include "Light\PointLight.h"
#include "Texture\Material.h"
#include "DescriptorHeap\CBVSRVUAVDescriptorHeap.h"

namespace library
{
    /*
        Renderer가 그리게 될 Scene
    */
    class Scene final
    {
    public:
        Scene();
        Scene(const Scene& other) = delete;
        Scene(Scene&& other) = delete;
        Scene& operator=(const Scene& other) = delete;
        Scene& operator=(Scene&& other) = delete;
        ~Scene() = default;

        HRESULT Initialize(
            _In_ const ComPtr<ID3D12Device>& pDevice,
            _In_ const ComPtr<ID3D12CommandQueue>& pCommandQueue,
            _In_ CBVSRVUAVDescriptorHeap& cbvSrvUavDescriptorHeap
        );
        void AddRenderable(_In_ const std::shared_ptr<Renderable>& pRenderable);
        void AddModel(_In_ const std::shared_ptr<Model>& pModel);
        const std::vector<std::shared_ptr<Renderable>>& GetRenderables() const;
        void AddLight(_In_ const std::shared_ptr<PointLight>& pLight);
        void AddMaterial(_In_ const std::shared_ptr<Material>& pMaterial);
        void Update(_In_ FLOAT deltaTime);
        ComPtr<ID3D12Resource>& GetPointLightsConstantBuffer();
    private:
        HRESULT createLightConstantBuffer(_In_ const ComPtr<ID3D12Device>& pDevice);
        void updateLightConstantBuffer();
    private:
        std::vector<std::shared_ptr<Renderable>> m_renderables;
        std::vector<std::shared_ptr<Model>> m_models;
        std::vector<std::shared_ptr<PointLight>> m_lights;  
        std::vector<std::shared_ptr<Material>> m_materials;
        ComPtr<ID3D12Resource> m_pointLightsConstantBuffer;
        void* m_pointLightMappedData;
    };
}