
#include "pch_game.h"
#include "Scene\Scene.h"
#include "Game\Game.h"
#include "Scene\CustomScene.h"


INT WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ INT nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);	
	std::unique_ptr<library::Game> game = std::make_unique<library::Game>(L"D3D12 ����Ʈ���̽�",800,600);
	std::shared_ptr<library::Scene> customScene = std::make_shared<CustomScene>();
	game->GetRenderer()->SetMainScene(customScene);//���ӿ��� ����� Scene����
	if (FAILED(game->Initialize(hInstance, nCmdShow)))
	{
		return 0;
	}
	return game->Run();
}


