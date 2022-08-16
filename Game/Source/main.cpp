
#include "pch.h"
#include "Scene\Scene.h"
#include "Game\Game.h"
#include "Cube\BaseCube.h"
#include "Light\RotatingLight.h"
#include "Texture\Texture.h"
#include <DirectXColors.h>

INT WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ INT nCmdShow)
{
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);
	std::unique_ptr<library::Game> game = std::make_unique<library::Game>(L"D3D12 레이트레이싱 Texturing테스트",1920,1080);
	std::shared_ptr<library::Scene> scene = std::make_shared<library::Scene>();
	
	XMFLOAT4 color;
	XMStoreFloat4(&color, Colors::Azure);
	std::shared_ptr<library::Renderable> cube1 = std::make_shared<BaseCube>(
		XMVectorSet(0.5f, 0.6f, 0.f, 1.0f),
		XMVectorSet(0.f, 0.f, 0.f, 1.0f),
		XMVectorSet(0.3f, 0.3f, 0.3f, 1.f),
		color
	);

	XMStoreFloat4(&color, Colors::Aquamarine);
	std::shared_ptr<library::Renderable> cube2 = std::make_shared<BaseCube>(
		XMVectorSet(-0.5f, 0.6f, 0.f, 1.0f),
		XMVectorSet(0.f, 0.f, 0.f, 1.0f),
		XMVectorSet(0.3f, 0.3f, 0.3f, 1.f),
		color
	);

	XMStoreFloat4(&color, Colors::Aqua);
	std::shared_ptr<library::Renderable> plane = std::make_shared<BaseCube>(
		XMVectorSet(0.f, -0.3f, 0.f, 1.0f),
		XMVectorSet(0.f, 0.f, 0.f, 1.0f),
		XMVectorSet(5.f, 0.1f, 5.f, 1.f),
		color
	);
	std::shared_ptr<library::PointLight> light1 = std::make_shared<RotatingLight>(XMVectorSet(0.f, 5.f, -5.f,1.f));

	std::shared_ptr<library::Material> material1 = std::make_shared<library::Material>();
	material1->SetDiffuseTexture(std::make_shared<library::Texture>(L"Assets/Texture/seafloor.dds"));
	cube1->SetMaterial(material1);
	cube2->SetMaterial(material1);
	plane->SetMaterial(material1);

	scene->AddRenderable(cube1);
	scene->AddRenderable(cube2);
	scene->AddRenderable(plane);
	scene->AddLight(light1);
	scene->AddMaterial(material1);
	game->GetRenderer()->SetMainScene(scene);//게임에서 사용할 Scene선택
	if (FAILED(game->Initialize(hInstance, nCmdShow)))
	{
		return 0;
	}
	return game->Run();
}


