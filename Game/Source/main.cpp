

#include "Common/Common.h"
#include "Scene\Scene.h"
#include "Game/Game.h"
#include "TestTriangle.h"

INT WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ INT nCmdShow)
{
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);
	std::unique_ptr<library::Game> game = std::make_unique<library::Game>(L"D3D12 TLAS,BLAS�߻�ȭ �׽�Ʈ",1920,1080);
	std::shared_ptr<library::Scene> scene = std::make_shared<library::Scene>();
	std::shared_ptr<library::Renderable> testTriangle = std::make_shared<TestTriangle>();//�׽�Ʈ �ﰢ��
	
	scene->AddRenderable(testTriangle);//���� �ﰢ�� �߰�
	game->GetRenderer()->SetMainScene(scene);//���ӿ��� ����� Scene����
	
	if (FAILED(game->Initialize(hInstance, nCmdShow)))
	{
		return 0;
	}
	return game->Run();
}


