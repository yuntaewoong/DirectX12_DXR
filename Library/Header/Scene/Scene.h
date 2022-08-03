#pragma once

#include "Common/Common.h"
#include "Render\Renderable.h"
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
        void AddRenderable(_In_ std::shared_ptr<Renderable>& pRenderable);
        std::vector<std::shared_ptr<Renderable>>& GetRenderables();
    private:
        std::vector<std::shared_ptr<Renderable>> m_renderables;
    };
}