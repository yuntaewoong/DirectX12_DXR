

#include "Common/Common.h"
#include "Scene\Scene.h"
#include "Game/Game.h"
#include "TestTriangle.h"

INT WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ INT nCmdShow)
{
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);
	std::unique_ptr<library::Game> game = std::make_unique<library::Game>(L"D3D12 TLAS Instance Buffer테스트",1920,1080);
	std::shared_ptr<library::Scene> scene = std::make_shared<library::Scene>();
	std::shared_ptr<library::Renderable> testTriangle1 = std::make_shared<TestTriangle>(
		XMVectorSet(0.5f,0.f,0.f,1.f),
		XMVectorSet(0.f,0.f,0.f,1.f),
		XMVectorSet(0.7f,0.7f,0.7f,1.f)
	);//테스트 삼각형
	
	std::shared_ptr<library::Renderable> testTriangle2 = std::make_shared<TestTriangle>(
		XMVectorSet(-0.5f, 0.f, 0.f, 1.f),
		XMVectorSet(0.f, 0.f, 0.f, 1.f),
		XMVectorSet(0.5f, 0.5f, 0.5f, 1.f)
	);//테스트 삼각형

	scene->AddRenderable(testTriangle1);//씬에 삼각형 추가
	scene->AddRenderable(testTriangle2);//씬에 삼각형 추가
	game->GetRenderer()->SetMainScene(scene);//게임에서 사용할 Scene선택
	if (FAILED(game->Initialize(hInstance, nCmdShow)))
	{
		return 0;
	}
	return game->Run();
}


