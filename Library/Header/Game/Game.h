#pragma once

#include "Common/Common.h"
#include "Window/MainWindow.h"
#include "Render/Renderer.h"

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
        std::unique_ptr<Renderer>& GetRenderer();
        PCWSTR GetGameName() const;
        std::unique_ptr<MainWindow>& GetWindow();
    private:
        PCWSTR m_pszGameName;
        std::unique_ptr<MainWindow> m_mainWindow;
        std::unique_ptr<Renderer> m_renderer;
    };
}