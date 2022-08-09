#include "Game/Game.h"

namespace library
{
	Game::Game(_In_ PCWSTR pszGameName, _In_ INT nWidth, _In_ INT nHeight) :
		m_pszGameName(pszGameName),
		m_mainWindow(std::make_unique<MainWindow>(nWidth,nHeight)),
		m_renderer(std::make_unique<RaytracingRenderer>())
	{ }

	/*
		Window와 Renderer의 Initilizing
	*/
	HRESULT Game::Initialize(_In_ HINSTANCE hInstance, _In_ INT nCmdShow)
	{
		HRESULT hr = m_mainWindow->Initialize(hInstance, nCmdShow, m_pszGameName);
		if (FAILED(hr))
			return hr;
		m_mainWindow->GetWindow();
		hr = m_renderer->Initialize(m_mainWindow->GetWindow());
		if (FAILED(hr))
			return hr;
		return S_OK;
	}

	/*
		게임 루프 시작
	*/
	INT Game::Run()
	{
		MSG msg = { 0 };
		LARGE_INTEGER StartingTime{
			.QuadPart = static_cast<LONGLONG>(0),
		};
		LARGE_INTEGER EndingTime{
			.QuadPart = static_cast<LONGLONG>(0),
		};
		LARGE_INTEGER Frequency{
			.QuadPart = static_cast<LONGLONG>(0),
		};
		QueryPerformanceFrequency(&Frequency);
		QueryPerformanceCounter(&StartingTime);
		FLOAT elapsedTime = 0.0f;
		while (WM_QUIT != msg.message)
		{
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				//end timer
				QueryPerformanceCounter(&EndingTime);
				elapsedTime = (EndingTime.QuadPart - StartingTime.QuadPart) / (FLOAT)Frequency.QuadPart;
				m_renderer->HandleInput(m_mainWindow->GetDirections(), m_mainWindow->GetMouseRelativeMovement(), elapsedTime);
				m_mainWindow->ResetMouseMovement();
				m_renderer->Update(elapsedTime);
				//start timer
				QueryPerformanceCounter(&StartingTime);
				m_renderer->Render();
			}
		}

		return static_cast<INT>(msg.wParam);
	}
	std::unique_ptr<RaytracingRenderer>& Game::GetRenderer()
	{
		return m_renderer;
	}
	PCWSTR Game::GetGameName() const
	{
		return m_pszGameName;
	}
}