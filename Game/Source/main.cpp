

#include "Common/Common.h"
#include "Game/Game.h"

INT WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ INT nCmdShow)
{
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);
	std::unique_ptr<library::Game> game = std::make_unique<library::Game>(L"D3D12 초기화 테스트",1920,1080);
	
	if (FAILED(game->Initialize(hInstance, nCmdShow)))
	{
		return 0;
	}
	return game->Run();
}


