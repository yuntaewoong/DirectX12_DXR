﻿#pragma once

#include "Common/Common.h"

#include "Window/BaseWindow.h"

namespace library
{
    /*
    class: MainWindow
    메인이 되는 윈도우
    메인윈도우 만의 HandleMessage, Initialize를 오버라이딩
    */
    class MainWindow : public BaseWindow<MainWindow>
    {
    public:
        MainWindow(INT nWidth,INT nHeight);
        MainWindow(const MainWindow& other) = delete;
        MainWindow(MainWindow&& other) = delete;
        MainWindow& operator=(const MainWindow& other) = delete;
        MainWindow& operator=(MainWindow&& other) = delete;
        virtual ~MainWindow() = default;

        HRESULT Initialize(_In_ HINSTANCE hInstance, _In_ INT nCmdShow, _In_ PCWSTR pszWindowName) override;
        PCWSTR GetWindowClassName() const override;
        LRESULT HandleMessage(_In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam) override;
        
        const DirectionsInput& GetDirections() const;
        const MouseRelativeMovement& GetMouseRelativeMovement() const;
        void ResetMouseMovement();
        INT GetWidth() const;
        INT GetHeight() const;
    private:
        INT m_nWidth;
        INT m_nHeight;
        DirectionsInput m_directions;//키보드 인풋
        MouseRelativeMovement m_mouseRelativeMovement;//마우스 인풋
    };
}

