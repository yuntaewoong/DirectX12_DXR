
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
	std::unique_ptr<library::Game> game = std::make_unique<library::Game>(L"D3D12 ����Ʈ���̽� Reflection�׽�Ʈ",1920,1080);
	std::shared_ptr<library::Scene> scene = std::make_shared<library::Scene>();
	
	XMFLOAT4 color;
	XMStoreFloat4(&color, Colors::White);
	std::shared_ptr<library::Renderable> cube1 = std::make_shared<BaseCube>(//ť��1
		XMVectorSet(0.5f, 0.6f, 0.f, 1.0f),
		XMVectorSet(0.f, 0.f, 0.f, 1.0f),
		XMVectorSet(0.3f, 0.3f, 0.3f, 1.f),
		color
	);

	XMStoreFloat4(&color, Colors::White);
	std::shared_ptr<library::Renderable> cube2 = std::make_shared<BaseCube>(//ť��2
		XMVectorSet(-0.5f, 0.6f, 0.f, 1.0f),
		XMVectorSet(0.f, 0.f, 0.f, 1.0f),
		XMVectorSet(0.3f, 0.3f, 0.3f, 1.f),
		color
	);

	XMStoreFloat4(&color, Colors::Coral);
	std::shared_ptr<library::Renderable> plane = std::make_shared<BasePlane>(//�ٴ�
		XMVectorSet(0.f, -0.3f, 0.f, 1.0f),
		XMVectorSet(0.f, 0.f, 0.f, 1.0f),
		XMVectorSet(5.f, 1.f, 5.f, 1.f),
		color
	);

	XMStoreFloat4(&color, Colors::White);
	std::shared_ptr<library::Renderable> mirror = std::make_shared<BasePlane>(//�ſ�
		XMVectorSet(0.f, 0.6f, 2.5f, 1.0f),
		XMVectorSet(0.f, XM_PIDIV2, XM_PIDIV2, 1.0f),
		XMVectorSet(5.f, 1.f, 5.f, 1.f),
		color
	);

	std::shared_ptr<library::PointLight> light1 = std::make_shared<RotatingLight>(XMVectorSet(0.f, 5.f, -5.f,1.f));
	
	std::filesystem::path projectDirPath;
	{
		std::string projectDirString = EXPAND(PROJECT_DIR);//project_dir�� ���ڿ�ȭ
		projectDirString.erase(0, 1);//Root����
		projectDirString.erase(projectDirString.size() - 2);// "\."����
		projectDirPath = projectDirString;
	}
	std::shared_ptr<library::Material> floorMaterial = std::make_shared<library::Material>();//�ٴ� �ؽ�ó
	//std::filesystem::path floorTexturePath(L"Assets/Texture/seafloor.dds");//project dir�󿡼��� ���Path
	
	std::shared_ptr<library::Material> woodMaterial = std::make_shared<library::Material>();//���� �ؽ�ó
	std::filesystem::path woodTexturePath(L"Assets/Texture/wood.jpg");//project dir�󿡼��� ���Path
	
	std::shared_ptr<library::Material> mirrorMaterial = std::make_shared<library::Material>();//Texture���� Material

	{//Material�� �ݻ�Ǵ� ���� ����(�⺻���� 0)
		mirrorMaterial->SetReflectivity(0.9f);
		floorMaterial->SetReflectivity(0.6f);
		woodMaterial->SetReflectivity(0.1f);
	}

	{//Material=>Diffuse Texture���� ����
		//floorMaterial->SetDiffuseTexture(std::make_shared<library::Texture>(projectDirPath / floorTexturePath));
		woodMaterial->SetDiffuseTexture(std::make_shared<library::Texture>(projectDirPath / woodTexturePath));
	}
	{//Scene���� �ʱ�ȭ���� Object�� Pass
		scene->AddRenderable(cube1);
		scene->AddRenderable(cube2);
		scene->AddRenderable(plane);
		scene->AddRenderable(mirror);

		scene->AddLight(light1);
		scene->AddMaterial(floorMaterial);
		scene->AddMaterial(woodMaterial);
		scene->AddMaterial(mirrorMaterial);
	}
	{//Renderable=>Material ���� ����
		cube1->SetMaterial(woodMaterial);
		cube2->SetMaterial(woodMaterial);
		plane->SetMaterial(floorMaterial);
		mirror->SetMaterial(mirrorMaterial);
	}
	game->GetRenderer()->SetMainScene(scene);//���ӿ��� ����� Scene����
	if (FAILED(game->Initialize(hInstance, nCmdShow)))
	{
		return 0;
	}
	return game->Run();
}


