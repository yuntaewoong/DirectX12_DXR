
#include "pch_game.h"
#include "Scene\Scene.h"
#include "Game\Game.h"
#include "Cube\BaseCube.h"
#include "Plane\BasePlane.h"
#include "Light\RotatingLight.h"
#include "Texture\Texture.h"
#include <DirectXColors.h>

#define STRINGIFY(x) #x

#define EXPAND(x) STRINGIFY(x)

INT WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ INT nCmdShow)
{
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);
	std::unique_ptr<library::Game> game = std::make_unique<library::Game>(L"D3D12 레이트레이싱 Reflection테스트",1920,1080);
	std::shared_ptr<library::Scene> scene = std::make_shared<library::Scene>();
	
	XMFLOAT4 color;
	XMStoreFloat4(&color, Colors::White);
	std::shared_ptr<library::Renderable> cube1 = std::make_shared<BaseCube>(//큐브1
		XMVectorSet(0.5f, 0.6f, 0.f, 1.0f),
		XMVectorSet(0.f, 0.f, 0.f, 1.0f),
		XMVectorSet(0.3f, 0.3f, 0.3f, 1.f),
		color
	);

	XMStoreFloat4(&color, Colors::White);
	std::shared_ptr<library::Renderable> cube2 = std::make_shared<BaseCube>(//큐브2
		XMVectorSet(-0.5f, 0.6f, 0.f, 1.0f),
		XMVectorSet(0.f, 0.f, 0.f, 1.0f),
		XMVectorSet(0.3f, 0.3f, 0.3f, 1.f),
		color
	);

	XMStoreFloat4(&color, Colors::Coral);
	std::shared_ptr<library::Renderable> plane = std::make_shared<BasePlane>(//바닥
		XMVectorSet(0.f, -0.3f, 0.f, 1.0f),
		XMVectorSet(0.f, 0.f, 0.f, 1.0f),
		XMVectorSet(5.f, 1.f, 5.f, 1.f),
		color
	);

	XMStoreFloat4(&color, Colors::White);
	std::shared_ptr<library::Renderable> mirror = std::make_shared<BasePlane>(//거울
		XMVectorSet(0.f, 0.6f, 2.5f, 1.0f),
		XMVectorSet(0.f, XM_PIDIV2, XM_PIDIV2, 1.0f),
		XMVectorSet(5.f, 1.f, 5.f, 1.f),
		color
	);

	std::shared_ptr<library::PointLight> light1 = std::make_shared<RotatingLight>(XMVectorSet(0.f, 5.f, -5.f,1.f));
	
	std::filesystem::path projectDirPath;
	{
		std::string projectDirString = EXPAND(PROJECT_DIR);//project_dir의 문자열화
		projectDirString.erase(0, 1);//Root제거
		projectDirString.erase(projectDirString.size() - 2);// "\."제거
		projectDirPath = projectDirString;
	}
	std::shared_ptr<library::Material> floorMaterial = std::make_shared<library::Material>();//바닥 텍스처
	//std::filesystem::path floorTexturePath(L"Assets/Texture/seafloor.dds");//project dir상에서의 상대Path
	
	std::shared_ptr<library::Material> woodMaterial = std::make_shared<library::Material>();//목재 텍스처
	std::filesystem::path woodTexturePath(L"Assets/Texture/wood.jpg");//project dir상에서의 상대Path
	
	std::shared_ptr<library::Material> mirrorMaterial = std::make_shared<library::Material>();//Texture없는 Material

	{//Material의 반사되는 정도 세팅(기본값은 0)
		mirrorMaterial->SetReflectivity(0.9f);
		floorMaterial->SetReflectivity(0.6f);
		woodMaterial->SetReflectivity(0.1f);
	}

	{//Material=>Diffuse Texture대응 세팅
		//floorMaterial->SetDiffuseTexture(std::make_shared<library::Texture>(projectDirPath / floorTexturePath));
		woodMaterial->SetDiffuseTexture(std::make_shared<library::Texture>(projectDirPath / woodTexturePath));
	}
	{//Scene에서 초기화해줄 Object들 Pass
		scene->AddRenderable(cube1);
		scene->AddRenderable(cube2);
		scene->AddRenderable(plane);
		scene->AddRenderable(mirror);

		scene->AddLight(light1);
		scene->AddMaterial(floorMaterial);
		scene->AddMaterial(woodMaterial);
		scene->AddMaterial(mirrorMaterial);
	}
	{//Renderable=>Material 대응 세팅
		cube1->SetMaterial(woodMaterial);
		cube2->SetMaterial(woodMaterial);
		plane->SetMaterial(floorMaterial);
		mirror->SetMaterial(mirrorMaterial);
	}
	game->GetRenderer()->SetMainScene(scene);//게임에서 사용할 Scene선택
	if (FAILED(game->Initialize(hInstance, nCmdShow)))
	{
		return 0;
	}
	return game->Run();
}


