#include "Window/MainWindow.h"

namespace library
{
    MainWindow::MainWindow(INT nWidth, INT nHeight) :
        m_nWidth(nWidth),
        m_nHeight(nHeight)
    {}
    
    HRESULT MainWindow::Initialize(_In_ HINSTANCE hInstance, _In_ INT nCmdShow, _In_ PCWSTR pszWindowName)
    {
        RECT rc = { 0, 0, m_nWidth, m_nHeight };
        AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
        HRESULT hr = initialize(
            hInstance,
            nCmdShow,
            pszWindowName,
            WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT,
            rc.right - rc.left,
            rc.bottom - rc.top,
            nullptr,
            nullptr
        );
        if (FAILED(hr))
        {
            return hr;
        }
        return S_OK;
    }
    PCWSTR MainWindow::GetWindowClassName() const
    {
        return L"MainWindow";
    }
    LRESULT MainWindow::HandleMessage(_In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
    {
        PAINTSTRUCT ps;
        HDC hdc;

        switch (uMsg)
        {
        case WM_PAINT:
            hdc = BeginPaint(m_hWnd, &ps);
            EndPaint(m_hWnd, &ps);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        case WM_KEYDOWN:
            break;
        default:
            return DefWindowProc(m_hWnd, uMsg, wParam, lParam);
        }

        return 0;
    }
}