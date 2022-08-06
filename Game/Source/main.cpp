

#include "Common/Common.h"
#include "Scene\Scene.h"
#include "Game/Game.h"
#include "Cube\BaseCube.h"

INT WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ INT nCmdShow)
{
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);
	std::unique_ptr<library::Game> game = std::make_unique<library::Game>(L"D3D12 ����Ʈ���̽� 3D CUBE ������",1920,1080);
	std::shared_ptr<library::Scene> scene = std::make_shared<library::Scene>();
	std::shared_ptr<library::Renderable> cube1 = std::make_shared<BaseCube>(
		XMVectorSet(0.5f, 0.f, 2.f, 1.0f),
		XMVectorSet(0.f, 0.f, 0.f, 1.0f),
		XMVectorSet(0.3f, 0.3f, 0.3f, 0.3f)
	);

	std::shared_ptr<library::Renderable> cube2 = std::make_shared<BaseCube>(
		XMVectorSet(-0.5f, 0.f, 2.f, 1.0f),
		XMVectorSet(0.f, 0.f, 0.f, 1.0f),
		XMVectorSet(0.3f, 0.3f, 0.3f, 0.3f)
		);

	scene->AddRenderable(cube1);
	scene->AddRenderable(cube2);
	game->GetRenderer()->SetMainScene(scene);//���ӿ��� ����� Scene����
	if (FAILED(game->Initialize(hInstance, nCmdShow)))
	{
		return 0;
	}
	return game->Run();
}


