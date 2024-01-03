
#include "pch_game.h"
#include "Scene\Scene.h"
#include "Game\Game.h"
#include "Scene\CustomScene.h"
#include "Scene\CornellBoxScene.h"
#include "Scene\BathroomScene.h"


INT WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ INT nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);	
	std::unique_ptr<library::Game> game = std::make_unique<library::Game>(L"D3D12 레이트레이싱",800,600);
	std::shared_ptr<library::Scene> bathroomScene = std::make_shared<BathroomScene>();
	std::shared_ptr<library::Scene> cornellBoxScene = std::make_shared<CornellBoxScene>();
	std::shared_ptr<library::Scene> customScene = std::make_shared<CustomScene>();
	{//게임에서 사용할 Scene선택
		//game->GetRenderer()->SetMainScene(bathroomScene);
		//game->GetRenderer()->SetMainScene(cornellBoxScene);
		game->GetRenderer()->SetMainScene(customScene);
	}
	if (FAILED(game->Initialize(hInstance, nCmdShow)))
	{
		return 1;
	}
	return game->Run();
}


