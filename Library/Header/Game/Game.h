#pragma once

#include "Common/Common.h"
#include "Window/MainWindow.h"
#include "Render/RaytracingRenderer.h"
#include "Scene\Scene.h"

namespace library
{
    /*
        Renderer와 window를 관리하는 wrapper클래스
    */
    class Game final
    {
    public:
        Game(_In_ PCWSTR pszGameName, _In_ INT nWidth, _In_ INT nHeight);
        Game(const Game& other) = delete;
        Game(Game&& other) = delete;
        Game& operator=(const Game& other) = delete;
        Game& operator=(Game&& other) = delete;
        ~Game() = default;

        HRESULT Initialize(_In_ HINSTANCE hInstance, _In_ INT nCmdShow);
        INT Run();
        std::unique_ptr<RaytracingRenderer>& GetRenderer();
        PCWSTR GetGameName() const;
        std::unique_ptr<MainWindow>& GetWindow();

        void AddScene(_In_ const std::shared_ptr<Scene>& scene);
        std::shared_ptr<Scene> GetCurrentScene() const;
    private:
        void ChangeScene(_In_ UINT sceneIndex);
    private:
        PCWSTR m_pszGameName;
        std::unique_ptr<MainWindow> m_mainWindow;
        std::unique_ptr<RaytracingRenderer> m_renderer;
        std::vector<std::shared_ptr<Scene>> m_scenes;
        UINT m_currentSceneIndex;
        UINT m_pastFrameSceneIndex;
        
    };
}