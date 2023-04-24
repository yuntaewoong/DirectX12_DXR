
#include "pch_game.h"
#include "Scene\Scene.h"
#include "Game\Game.h"
#include "Cube\BaseCube.h"
#include "Plane\BasePlane.h"
#include "Light\RotatingLight.h"
#include "Texture\Texture.h"
//#include "Model\Model.h"
#include <DirectXColors.h>

#define STRINGIFY(x) #x

#define EXPAND(x) STRINGIFY(x)

INT WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ INT nCmdShow)
{
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);
	std::unique_ptr<library::Game> game = std::make_unique<library::Game>(L"D3D12 ����Ʈ���̽�",800,600);
	std::shared_ptr<library::Scene> scene = std::make_shared<library::Scene>();
	
	XMFLOAT4 color;
	XMStoreFloat4(&color, Colors::White);
	std::shared_ptr<library::Mesh> cube1 = std::make_shared<BaseCube>(//ť��1
		XMVectorSet(0.6f, 0.0f, 0.f, 1.0f),
		XMVectorSet(0.f, 0.f, 0.f, 1.0f),
		XMVectorSet(0.3f, 0.3f, 0.3f, 1.f),
		color
	);

	XMStoreFloat4(&color, Colors::White);
	std::shared_ptr<library::Mesh> cube2 = std::make_shared<BaseCube>(//ť��2
		XMVectorSet(-0.6f, 0.0f, 0.f, 1.0f),
		XMVectorSet(0.f, 0.f, 0.f, 1.0f),
		XMVectorSet(0.3f, 0.3f, 0.3f, 1.f),
		color
	);

	XMStoreFloat4(&color, Colors::White);
	std::shared_ptr<library::Mesh> cube3 = std::make_shared<BaseCube>(//ť��3
		XMVectorSet(0.0f, 0.6f, 0.f, 1.0f),
		XMVectorSet(0.f, 0.f, 0.f, 1.0f),
		XMVectorSet(0.3f, 0.3f, 0.3f, 1.f),
		color
	);

	XMStoreFloat4(&color, Colors::White);
	std::shared_ptr<library::Mesh> cube4 = std::make_shared<BaseCube>(//ť��4
		XMVectorSet(0.0f, 0.0f, 0.6f, 1.0f),
		XMVectorSet(0.f, 0.f, 0.f, 1.0f),
		XMVectorSet(0.3f, 0.3f, 0.3f, 1.f),
		color
	);

	XMStoreFloat4(&color, Colors::White);
	std::shared_ptr<library::Mesh> cube5 = std::make_shared<BaseCube>(//ť��5
		XMVectorSet(0.0f, 0.0f, -0.6f, 1.0f),
		XMVectorSet(0.f, 0.f, 0.f, 1.0f),
		XMVectorSet(0.3f, 0.3f, 0.3f, 1.f),
		color
	);

	XMStoreFloat4(&color, Colors::Coral);
	std::shared_ptr<library::Mesh> plane = std::make_shared<BasePlane>(//�ٴ�
		XMVectorSet(0.f, -0.3f, 0.f, 1.0f),
		XMVectorSet(0.f, 0.f, 0.f, 1.0f),
		XMVectorSet(5.f, 1.f, 5.f, 1.f),
		color
	);

	XMStoreFloat4(&color, Colors::White);
	std::shared_ptr<library::Mesh> mirror = std::make_shared<BasePlane>(//�ſ�
		XMVectorSet(0.f, 0.6f, 2.5f, 1.0f),
		XMVectorSet(0.f, XM_PIDIV2, XM_PIDIV2, 1.0f),
		XMVectorSet(5.f, 1.f, 5.f, 1.f),
		color
	);

	XMStoreFloat4(&color, Colors::White);
	std::shared_ptr<library::Model> cyborg = std::make_shared<library::Model>(//���̺��� ��
		L"Assets/Model/cyborg/cyborg.obj",
		XMVectorSet(0.f, -1.f, 0.f, 1.0f),
		XMVectorSet(0.f, 0.f, 0.f, 1.0f),
		XMVectorSet(1.f, 1.f, 1.f, 1.f),
		color
	);

	XMStoreFloat4(&color, Colors::White);
	std::shared_ptr<library::Model> sphere = std::make_shared<library::Model>(//�� ��
		L"Assets/Model/sphere/source/sphere.fbx",
		XMVectorSet(3.f, 3.f, 0.f, 1.0f),
		XMVectorSet(0.f, 0.f, 0.f, 1.0f),
		XMVectorSet(0.1f, 0.1f, 0.1f, 1.0f),
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
	std::filesystem::path floorTexturePath(L"Assets/Texture/seafloor.dds");//project dir�󿡼��� ���Path
	
	std::shared_ptr<library::Material> woodMaterial = std::make_shared<library::Material>();//���� �ؽ�ó
	std::filesystem::path woodTexturePath(L"Assets/Texture/wood.jpg");//project dir�󿡼��� ���Path
	
	std::shared_ptr<library::Material> mirrorMaterial = std::make_shared<library::Material>();//Texture���� Material

	std::shared_ptr<library::Material> ironPBRMaterial = std::make_shared<library::Material>();//PBR ö
	std::filesystem::path ironBaseColorTexturePath(L"Assets/Texture/IronPBRTexture/rustediron2_basecolor.png");//project dir�󿡼��� ���Path
	std::filesystem::path ironNormalTexturePath(L"Assets/Texture/IronPBRTexture/rustediron2_normal.png");//project dir�󿡼��� ���Path
	std::filesystem::path ironRoughnessTexturePath(L"Assets/Texture/IronPBRTexture/rustediron2_roughness.png");//project dir�󿡼��� ���Path
	std::filesystem::path ironMetallicTexturePath(L"Assets/Texture/IronPBRTexture/rustediron2_metallic.png");//project dir�󿡼��� ���Path

	

	{//Material=>Texture���� ����
		woodMaterial->SetDiffuseTexture(std::make_shared<library::Texture>(projectDirPath / woodTexturePath));
		ironPBRMaterial->SetDiffuseTexture(std::make_shared<library::Texture>(projectDirPath / ironBaseColorTexturePath));
		ironPBRMaterial->SetNormalTexture(std::make_shared<library::Texture>(projectDirPath / ironNormalTexturePath));
		ironPBRMaterial->SetRoughnessTexture(std::make_shared<library::Texture>(projectDirPath / ironRoughnessTexturePath));
		ironPBRMaterial->SetMetallicTexture(std::make_shared<library::Texture>(projectDirPath / ironMetallicTexturePath));
	}

	{//Material�� �ݻ�Ǵ� ���� ����(�⺻���� 0)
		mirrorMaterial->SetReflectivity(0.9f);
		floorMaterial->SetReflectivity(0.6f);
		woodMaterial->SetReflectivity(0.0f);
	}

	{//Scene���� �ʱ�ȭ���� Object�� Pass
		scene->AddMesh(cube1);
		scene->AddMesh(cube2);
		scene->AddMesh(cube3);
		scene->AddMesh(cube4);
		scene->AddMesh(cube5);
		scene->AddMesh(plane);
		scene->AddMesh(mirror);

		scene->AddModel(cyborg);
		scene->AddModel(sphere);

		scene->AddLight(light1);
		scene->AddMaterial(floorMaterial);
		scene->AddMaterial(woodMaterial);
		scene->AddMaterial(mirrorMaterial);
		scene->AddMaterial(ironPBRMaterial);
	}
	{//Mesh=>Material ���� ����
		cube1->SetMaterial(woodMaterial);
		cube2->SetMaterial(woodMaterial);
		cube3->SetMaterial(woodMaterial);
		cube4->SetMaterial(woodMaterial);
		cube5->SetMaterial(ironPBRMaterial);
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


