
#include "pch_game.h"
#include "Scene\Scene.h"
#include "Game\Game.h"
#include "Cube\BaseCube.h"
#include "Plane\BasePlane.h"
#include "Light\RotatingLight.h"
#include "Texture\Texture.h"
#include "Model\Model.h"
#include <DirectXColors.h>

#define STRINGIFY(x) #x

#define EXPAND(x) STRINGIFY(x)

INT WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ INT nCmdShow)
{
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);
	std::unique_ptr<library::Game> game = std::make_unique<library::Game>(L"D3D12 레이트레이싱",800,600);
	std::shared_ptr<library::Scene> scene = std::make_shared<library::Scene>();
	
	XMFLOAT4 color;
	XMStoreFloat4(&color, Colors::White);
	std::shared_ptr<library::Mesh> cube1 = std::make_shared<BaseCube>(//큐브1
		XMVectorSet(0.6f, 0.0f, 0.f, 1.0f),
		XMVectorSet(0.f, 0.f, 0.f, 1.0f),
		XMVectorSet(0.3f, 0.3f, 0.3f, 1.f),
		color
	);

	XMStoreFloat4(&color, Colors::White);
	std::shared_ptr<library::Mesh> cube2 = std::make_shared<BaseCube>(//큐브2
		XMVectorSet(-0.6f, 0.0f, 0.f, 1.0f),
		XMVectorSet(0.f, 0.f, 0.f, 1.0f),
		XMVectorSet(0.3f, 0.3f, 0.3f, 1.f),
		color
	);

	XMStoreFloat4(&color, Colors::White);
	std::shared_ptr<library::Mesh> cube3 = std::make_shared<BaseCube>(//큐브3
		XMVectorSet(0.0f, 0.6f, 0.f, 1.0f),
		XMVectorSet(0.f, 0.f, 0.f, 1.0f),
		XMVectorSet(0.3f, 0.3f, 0.3f, 1.f),
		color
	);

	XMStoreFloat4(&color, Colors::White);
	std::shared_ptr<library::Mesh> cube4 = std::make_shared<BaseCube>(//큐브4
		XMVectorSet(0.0f, 0.0f, 0.6f, 1.0f),
		XMVectorSet(0.f, 0.f, 0.f, 1.0f),
		XMVectorSet(0.3f, 0.3f, 0.3f, 1.f),
		color
	);

	XMStoreFloat4(&color, Colors::White);
	std::shared_ptr<library::Mesh> cube5 = std::make_shared<BaseCube>(//큐브5
		XMVectorSet(0.0f, 0.0f, -0.6f, 1.0f),
		XMVectorSet(0.f, 0.f, 0.f, 1.0f),
		XMVectorSet(0.3f, 0.3f, 0.3f, 1.f),
		color
	);

	XMStoreFloat4(&color, Colors::Coral);
	std::shared_ptr<library::Mesh> plane = std::make_shared<BasePlane>(//바닥
		XMVectorSet(0.f, -0.3f, 0.f, 1.0f),
		XMVectorSet(0.f, 0.f, 0.f, 1.0f),
		XMVectorSet(5.f, 1.f, 5.f, 1.f),
		color
	);

	XMStoreFloat4(&color, Colors::White);
	std::shared_ptr<library::Mesh> mirror = std::make_shared<BasePlane>(//거울
		XMVectorSet(0.f, 0.6f, 2.5f, 1.0f),
		XMVectorSet(0.f, XM_PIDIV2, XM_PIDIV2, 1.0f),
		XMVectorSet(5.f, 1.f, 5.f, 1.f),
		color
	);

	XMStoreFloat4(&color, Colors::White);
	std::shared_ptr<library::Model> cyborg = std::make_shared<library::Model>(//사이보그 모델
		L"Assets/Model/cyborg/cyborg.obj",
		XMVectorSet(0.f, -1.f, 0.f, 1.0f),
		XMVectorSet(0.f, 0.f, 0.f, 1.0f),
		XMVectorSet(1.f, 1.f, 1.f, 1.f),
		color
	);

	XMStoreFloat4(&color, Colors::Red);
	std::shared_ptr<library::Model> pbrSpheres[7][7];
	for (UINT i = 0; i < 7u; i++)
	{
		for (UINT j = 0; j < 7u; j++)
		{
			pbrSpheres[i][j] = std::make_shared<library::Model>(
				L"Assets/Model/sphere/source/sphere.fbx",
				XMVectorSet(1.f + static_cast<float>(i)/1.4f, 1.f + static_cast<float>(j) /1.4f, 1.f, 1.0f),
				XMVectorSet(0.f, 0.f, 0.f, 1.0f),
				XMVectorSet(0.3f, 0.3f, 0.3f, 1.0f),
				color
			);
		}
	}
	
	


	std::shared_ptr<library::PointLight> light1 = std::make_shared<RotatingLight>(XMVectorSet(0.f, 5.f, -5.f,1.f));
	
	std::filesystem::path projectDirPath;
	{
		std::string projectDirString = EXPAND(PROJECT_DIR);//project_dir의 문자열화
		projectDirString.erase(0, 1);//Root제거
		projectDirString.erase(projectDirString.size() - 2);// "\."제거
		projectDirPath = projectDirString;
	}
	
	std::shared_ptr<library::Material> floorMaterial = std::make_shared<library::Material>(MaterialType::Phong);//바닥 텍스처
	std::filesystem::path floorTexturePath(L"Assets/Texture/seafloor.dds");//project dir상에서의 상대Path
	
	std::shared_ptr<library::Material> woodMaterial = std::make_shared<library::Material>(MaterialType::Phong);//목재 텍스처
	std::filesystem::path woodTexturePath(L"Assets/Texture/wood.jpg");//project dir상에서의 상대Path
	
	std::shared_ptr<library::Material> mirrorMaterial = std::make_shared<library::Material>(MaterialType::Phong);//Texture없는 Material
	
	std::shared_ptr<library::Material> ironPBRMaterial = std::make_shared<library::Material>(MaterialType::PBR);//PBR 철
	std::filesystem::path ironBaseColorTexturePath(L"Assets/Texture/IronPBRTexture/rustediron2_basecolor.png");//project dir상에서의 상대Path
	std::filesystem::path ironNormalTexturePath(L"Assets/Texture/IronPBRTexture/rustediron2_normal.png");//project dir상에서의 상대Path
	std::filesystem::path ironRoughnessTexturePath(L"Assets/Texture/IronPBRTexture/rustediron2_roughness.png");//project dir상에서의 상대Path
	std::filesystem::path ironMetallicTexturePath(L"Assets/Texture/IronPBRTexture/rustediron2_metallic.png");//project dir상에서의 상대Path

	std::shared_ptr<library::Material> pbrTestMaterials[7][7];
	for (UINT i = 0; i < 7u; i++)
	{
		for (UINT j = 0; j < 7u; j++)
		{//i가 0이면 roughness 0, j가 0이면 metallic 0
			pbrTestMaterials[i][j] = std::make_shared<library::Material>(MaterialType::PBR);
			pbrTestMaterials[i][j]->SetRoughness(i / 7.f);
			pbrTestMaterials[i][j]->SetMetallic(j / 7.f);
		}
	}



	{//Material=>Texture대응 세팅
		woodMaterial->SetDiffuseTexture(std::make_shared<library::Texture>(projectDirPath / woodTexturePath));
		ironPBRMaterial->SetDiffuseTexture(std::make_shared<library::Texture>(projectDirPath / ironBaseColorTexturePath));
		ironPBRMaterial->SetNormalTexture(std::make_shared<library::Texture>(projectDirPath / ironNormalTexturePath));
		ironPBRMaterial->SetRoughnessTexture(std::make_shared<library::Texture>(projectDirPath / ironRoughnessTexturePath));
		ironPBRMaterial->SetMetallicTexture(std::make_shared<library::Texture>(projectDirPath / ironMetallicTexturePath));
	}

	{//Material의 반사되는 정도 세팅(기본값은 0)
		mirrorMaterial->SetReflectivity(0.9f);
		floorMaterial->SetReflectivity(0.6f);
		woodMaterial->SetReflectivity(0.0f);
	}

	{//Scene에서 초기화해줄 Object들 Pass
		scene->AddMesh(cube1);
		scene->AddMesh(cube2);
		scene->AddMesh(cube3);
		scene->AddMesh(cube4);
		scene->AddMesh(cube5);
		scene->AddMesh(plane);
		scene->AddMesh(mirror);

		scene->AddModel(cyborg);
		for (UINT i = 0; i < 7u; i++)
		{
			for (UINT j = 0; j < 7u; j++)
			{
				scene->AddModel(pbrSpheres[i][j]);
			}
		}
		

		scene->AddLight(light1);
		scene->AddMaterial(floorMaterial);
		scene->AddMaterial(woodMaterial);
		scene->AddMaterial(mirrorMaterial);
		scene->AddMaterial(ironPBRMaterial);
	}
	{//Mesh=>Material 대응 세팅
		cube1->SetMaterial(woodMaterial);
		cube2->SetMaterial(woodMaterial);
		cube3->SetMaterial(woodMaterial);
		cube4->SetMaterial(woodMaterial);
		cube5->SetMaterial(ironPBRMaterial);
		plane->SetMaterial(floorMaterial);
		mirror->SetMaterial(mirrorMaterial);

		for (UINT i = 0; i < 7u; i++)
		{
			for (UINT j = 0; j < 7u; j++)
			{
				pbrSpheres[i][j]->ForceMaterial(pbrTestMaterials[i][j]);
			}
		}

	}
	game->GetRenderer()->SetMainScene(scene);//게임에서 사용할 Scene선택
	if (FAILED(game->Initialize(hInstance, nCmdShow)))
	{
		return 0;
	}
	return game->Run();
}


