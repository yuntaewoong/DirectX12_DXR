#pragma once
#include "Common/Common.h"
namespace library
{
    /*
    class : BaseWindow
    window들의 base
    virtual class로 BaseWindow클래스의 객체를 만들 수 없다.
    Template과 static 윈도우 프로시저를 사용해서 각각의 윈도우가 별개의 윈도우 프로시저를 선언할 수 있다. 
    */
    template <class DerivedType>
    class BaseWindow
    {
    public:
        static LRESULT CALLBACK WindowProc(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);

        BaseWindow();
        BaseWindow(const BaseWindow& rhs) = delete;
        BaseWindow(BaseWindow&& rhs) = delete;
        BaseWindow& operator=(const BaseWindow& rhs) = delete;
        BaseWindow& operator=(BaseWindow&& rhs) = delete;
        virtual ~BaseWindow() = default;

        virtual HRESULT Initialize(_In_ HINSTANCE hInstance, _In_ INT nCmdShow, _In_ PCWSTR pszWindowName) = 0;
        virtual PCWSTR GetWindowClassName() const = 0;
        virtual LRESULT HandleMessage(_In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam) = 0;

        HWND GetWindow() const;

    protected:
        HRESULT initialize(
            _In_ HINSTANCE hInstance,
            _In_ INT nCmdShow,
            _In_ PCWSTR pszWindowName,
            _In_ DWORD dwStyle,
            _In_opt_ INT x = CW_USEDEFAULT,
            _In_opt_ INT y = CW_USEDEFAULT,
            _In_opt_ INT nWidth = CW_USEDEFAULT,
            _In_opt_ INT nHeight = CW_USEDEFAULT,
            _In_opt_ HWND hWndParent = nullptr,
            _In_opt_ HMENU hMenu = nullptr
        );

        HINSTANCE m_hInstance;
        HWND m_hWnd;
        LPCWSTR m_pszWindowName;
    };
    /*
        윈도우 프로시저,
        static으로 선언한 이유는 윈도우 프로시저는 전역접근이 가능해야하기 때문임
        SetWindowLongPtr, GetWindowLongPtr매크로로 자식 window객체에 대한 데이터를 설정하고 가져옴
    */
    template<class DerivedType>
    LRESULT BaseWindow<DerivedType>::WindowProc(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
    {
        DerivedType* pThis = nullptr;
        if (uMsg == WM_NCCREATE)
        {
            CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*> (lParam);
            pThis = reinterpret_cast<DerivedType*> (pCreate->lpCreateParams);
            SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
            pThis->m_hWnd = hWnd;
        }
        else
        {
            pThis = reinterpret_cast<DerivedType*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
        }
        if (pThis)
            return pThis->HandleMessage(uMsg, wParam, lParam);
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    template <class DerivedType>
    BaseWindow<DerivedType>::BaseWindow() : 
        m_hInstance(nullptr),
        m_hWnd(nullptr),
        m_pszWindowName(L"Default")
    { }
    template <class DerivedType>
    HWND BaseWindow<DerivedType>::GetWindow() const
    {
        return m_hWnd;
    }
    /*
        초기화 함수.
        윈도우 클래스 생성, 윈도우 프로시저 등록, 윈도우 출력 기능 담당
    */
    template <class DerivedType>
    HRESULT BaseWindow<DerivedType>::initialize(_In_ HINSTANCE hInstance,
        _In_ INT nCmdShow,
        _In_ PCWSTR pszWindowName,
        _In_ DWORD dwStyle,
        _In_opt_ INT x,
        _In_opt_ INT y,
        _In_opt_ INT nWidth,
        _In_opt_ INT nHeight,
        _In_opt_ HWND hWndParent,
        _In_opt_ HMENU hMenu)
    {
        WNDCLASSEX wcex = {
            .cbSize = sizeof(WNDCLASSEX),
            .style = CS_HREDRAW | CS_VREDRAW,
            .lpfnWndProc = DerivedType::WindowProc,
            .cbClsExtra = 0,
            .cbWndExtra = 0,
            .hInstance = hInstance,
            .hIcon = nullptr,
            .hCursor = LoadCursor(nullptr, IDC_ARROW),
            .hbrBackground = (HBRUSH)(COLOR_WINDOW + 1),
            .lpszMenuName = nullptr,
            .lpszClassName = GetWindowClassName(),
            .hIconSm = nullptr
        };
        if (!RegisterClassEx(&wcex))
            return E_FAIL;
        m_pszWindowName = pszWindowName;
        m_hInstance = hInstance;
        m_hWnd = CreateWindow(
            GetWindowClassName(),
            m_pszWindowName,
            dwStyle,
            x,
            y,
            nWidth,
            nHeight,
            hWndParent,
            hMenu,
            hInstance,
            this
        );
        if (!m_hWnd)
            return E_FAIL;

        ShowWindow(m_hWnd, nCmdShow);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        ImGui_ImplWin32_Init(m_hWnd);

        return S_OK;
    }
}