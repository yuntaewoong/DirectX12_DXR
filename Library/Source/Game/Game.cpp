#include "pch.h"
#include "Game/Game.h"
namespace library
{
	Game::Game(_In_ PCWSTR pszGameName, _In_ INT nWidth, _In_ INT nHeight) :
		m_pszGameName(pszGameName),
		m_mainWindow(std::make_unique<MainWindow>(nWidth,nHeight)),
		m_renderer(std::make_unique<RaytracingRenderer>(static_cast<FLOAT>(nWidth) / nHeight)),
		m_scenes(std::vector<std::shared_ptr<Scene>>()),
		m_currentSceneIndex(0u),
		m_pastFrameSceneIndex(0u)
	{}

	/*
		Window와 Renderer의 Initilizing
	*/
	HRESULT Game::Initialize(_In_ HINSTANCE hInstance, _In_ INT nCmdShow)
	{
		HRESULT hr = m_mainWindow->Initialize(hInstance, nCmdShow, m_pszGameName);
		if (FAILED(hr))
			return hr;
		m_mainWindow->GetWindow();
		m_renderer->SetMainScene(m_scenes[m_currentSceneIndex]);
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
				static int RenderTypeCurrent = 0;
				static int SceneTypeCurrent = 0;
				//end timer
				QueryPerformanceCounter(&EndingTime);
				elapsedTime = (EndingTime.QuadPart - StartingTime.QuadPart) / (FLOAT)Frequency.QuadPart;
				m_renderer->HandleInput(m_mainWindow->GetDirections(), m_mainWindow->GetMouseRelativeMovement(), elapsedTime);
				m_mainWindow->ResetMouseMovement();
				m_renderer->Update(elapsedTime,RenderTypeCurrent);
				//start timer
				QueryPerformanceCounter(&StartingTime);

				ImGui_ImplDX12_NewFrame();
				ImGui_ImplWin32_NewFrame();
				ImGui::NewFrame();
				ImGui::GetIO().FontGlobalScale = 2.f;
				ImGui::Begin("Status");

				ImGui::Text("%.3f ms/Frame (%.1f FPS)",1000.0f / ImGui::GetIO().Framerate,ImGui::GetIO().Framerate);

				const char* SceneType[3] = {"Bathroom PBRT Scene","Cornell Box","My Custom Scene"};
				ImGui::Combo("Scene", &SceneTypeCurrent, SceneType, IM_ARRAYSIZE(SceneType));

				
				const char* RenderType[2] = {"Real Time Raytracing","Path Tracing"};
				ImGui::Combo("Render", &RenderTypeCurrent, RenderType, IM_ARRAYSIZE(RenderType));

				ImGui::Text("Samples Per Pixel : %d", m_renderer->GetCurrentSamplesPerPixel());
				ImGui::Text("Control : ");
				ImGui::Text("'Q','E' : rotate");
				ImGui::Text("'w','A','S','D' : move horizontally");
				ImGui::Text("'shift','space' : move vertically");

				ImGui::End();
				ImGui::Render();
				m_renderer->Render(RenderTypeCurrent);

				if (m_pastFrameSceneIndex != SceneTypeCurrent)
				{//씬 종류 변화가 있을때
					ChangeScene(SceneTypeCurrent);
					m_pastFrameSceneIndex = SceneTypeCurrent;
				}
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
	void Game::AddScene(_In_ const std::shared_ptr<Scene>& scene)
	{
		m_scenes.push_back(scene);
	}
	std::shared_ptr<Scene> Game::GetCurrentScene() const
	{
		return m_scenes[m_currentSceneIndex];
	}
	void Game::ChangeScene(_In_ UINT sceneIndex)
	{
		m_currentSceneIndex = sceneIndex;
		HRESULT hr = m_renderer->WaitForGPU();//메모리 해제 전에 GPU작업완료대기
		ImGui_ImplDX12_Shutdown();
		m_renderer = std::make_unique<RaytracingRenderer>(16.f/9.f);
		m_renderer->SetMainScene(m_scenes[m_currentSceneIndex]);//새로운 Renderer에 새로운 Scene세팅
		m_renderer->Initialize(m_mainWindow->GetWindow());//새로운 Renderer 초기화
	}
}