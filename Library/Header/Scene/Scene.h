#pragma once

#include "Common/Common.h"
#include "Render\Renderable.h"
#include "Light\PointLight.h"

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

        HRESULT Initialize(_In_ ID3D12Device* pDevice);
        void AddRenderable(_In_ const std::shared_ptr<Renderable>& pRenderable);
        const std::vector<std::shared_ptr<Renderable>>& GetRenderables() const;
        void AddLight(_In_ const std::shared_ptr<PointLight>& pLight);
        void Update(_In_ FLOAT deltaTime);
        ComPtr<ID3D12Resource>& GetPointLightsConstantBuffer();
    private:
        std::vector<std::shared_ptr<Renderable>> m_renderables;
        std::vector<std::shared_ptr<PointLight>> m_lights;  
        ComPtr<ID3D12Resource> m_pointLightsConstantBuffer;
        void* m_pointLightMappedData;
    };
}