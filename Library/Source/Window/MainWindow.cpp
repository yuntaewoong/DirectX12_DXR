#include "pch.h"
#include "Window/MainWindow.h"
#include "Imgui\imgui_impl_win32.h"

namespace library
{
    MainWindow::MainWindow(INT nWidth, INT nHeight) :
        m_nWidth(nWidth),
        m_nHeight(nHeight),
        m_directions(),
        m_mouseRelativeMovement()
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
        RECT clientRc = {
            .left = 0,
            .top = 0,
            .right = 0,
            .bottom = 0
        };
        POINT p1 = {
            .x = 0,
            .y = 0
        };
        POINT p2 = {
            .x = 0,
            .y = 0
        };
        GetClientRect(GetWindow(), &clientRc);
        p1.x = clientRc.left;
        p1.y = clientRc.top;
        p2.x = clientRc.right;
        p2.y = clientRc.bottom;

        ClientToScreen(GetWindow(), &p1);
        ClientToScreen(GetWindow(), &p2);

        clientRc.left = p1.x;
        clientRc.top = p1.y;
        clientRc.right = p2.x;
        clientRc.bottom = p2.y;
        //ClipCursor(&clientRc);//locking cursor

        //initialize mouse
        RAWINPUTDEVICE rid = {
            rid.usUsagePage = 0x01, //Mouse
            rid.usUsage = 0x02,
            rid.dwFlags = 0,
            rid.hwndTarget = NULL
        };
        if (RegisterRawInputDevices(&rid, 1, sizeof(rid)) == FALSE)
        {
            return E_FAIL;
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
        extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
        if (ImGui_ImplWin32_WndProcHandler(m_hWnd, uMsg, wParam, lParam))
            return true;
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
        {
            unsigned char keyChar = static_cast<unsigned char>(wParam);
            switch (keyChar)
            {
            case 'w':
            case 'W':
                m_directions.bFront = true;
                break;
            case 's':
            case 'S':
                m_directions.bBack = true;
                break;
            case 'a':
            case 'A':
                m_directions.bLeft = true;
                break;
            case 'd':
            case 'D':
                m_directions.bRight = true;
                break;
            case 'q':
            case 'Q':
                m_directions.bRotateLeft = true;
                break;
            case 'e':
            case 'E':
                m_directions.bRotateRight = true;
                break;
            case 16://shift키
                m_directions.bDown = true;
                break;
            case 32://space바
                m_directions.bUp = true;
                break;

            }

            break;
        }
        case WM_KEYUP:
        {
            unsigned char keyChar = static_cast<unsigned char>(wParam);
            switch (keyChar)
            {
            case 'w':
            case 'W':
                m_directions.bFront = false;
                break;
            case 's':
            case 'S':
                m_directions.bBack = false;
                break;
            case 'a':
            case 'A':
                m_directions.bLeft = false;
                break;
            case 'd':
            case 'D':
                m_directions.bRight = false;
                break;
            case 'q':
            case 'Q':
                m_directions.bRotateLeft = false;
                break;
            case 'e':
            case 'E':
                m_directions.bRotateRight = false;
                break;
            case 16://shift키
                m_directions.bDown = false;
                break;
            case 32://space바
                m_directions.bUp = false;
                break;
            }
            break;
        }
        //case WM_INPUT://mouse handling
        //{
        //    UINT dataSize = 0;
        //    GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, NULL, &dataSize, sizeof(RAWINPUTHEADER)); //Need to populate data size first
        //    if (dataSize > 0)
        //    {
        //        std::unique_ptr<BYTE[]> rawdata = std::make_unique<BYTE[]>(dataSize);
        //        if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, rawdata.get(), &dataSize, sizeof(RAWINPUTHEADER)) == dataSize)
        //        {
        //            RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(rawdata.get());
        //            if (raw->header.dwType == RIM_TYPEMOUSE)
        //            {
        //                m_mouseRelativeMovement.X = raw->data.mouse.lLastX;
        //                m_mouseRelativeMovement.Y = raw->data.mouse.lLastY;
        //            }
        //        }
        //    }
        //    return DefWindowProc(m_hWnd, uMsg, wParam, lParam);
        //}
        default:
            return DefWindowProc(m_hWnd, uMsg, wParam, lParam);
        }

        return 0;
    }
    const DirectionsInput& MainWindow::GetDirections() const
    {
        return m_directions;
    }
    const MouseRelativeMovement& MainWindow::GetMouseRelativeMovement() const
    {
        return m_mouseRelativeMovement;
    }
    void MainWindow::ResetMouseMovement()
    {
        m_mouseRelativeMovement.X = 0;
        m_mouseRelativeMovement.Y = 0;
    }
    INT MainWindow::GetWidth() const
    {
        return m_nWidth;
    }
    INT MainWindow::GetHeight() const
    {
        return m_nHeight;
    }
}