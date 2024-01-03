#include "pch_game.h"
#include "Scene\CustomScene.h"
#include "Cube\BaseCube.h"
#include "Plane\BasePlane.h"
#include "Light\RotatingLight.h"
#include "Light\FixedLight.h"
#include "Texture\Texture.h"
#include "Model\Model.h"
#include <DirectXColors.h>

CustomScene::CustomScene()
	:
	Scene::Scene()
{
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
		XMVectorSet(15.f, 1.f, 15.f, 1.f),
		color
	);

	XMStoreFloat4(&color, Colors::White);
	std::shared_ptr<library::Mesh> mirror = std::make_shared<BasePlane>(//�ſ�
		XMVectorSet(0.f, 0.6f, 2.5f, 1.0f),
		XMVectorSet(0.f, XM_PIDIV2, XM_PIDIV2, 1.0f),
		XMVectorSet(15.f, 1.f, 15.f, 1.f),
		color
	);

	XMStoreFloat4(&color, Colors::White);
	std::shared_ptr<library::Model> cyborg = std::make_shared<library::Model>(//���̺��� ��
		L"Assets/Model/cyborg/cyborg.obj",
		XMVectorSet(-3.f, 1.f, 0.f, 1.0f),
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
	std::shared_ptr<library::PointLight> light2 = std::make_shared<FixedLight>(XMVectorSet(3.f, 3.f, -5.f, 1.f));


	std::filesystem::path projectDirPath;
	{
		std::string projectDirString = EXPAND(PROJECT_DIR);//project_dir�� ���ڿ�ȭ
		projectDirString.erase(0, 1);//Root����
		projectDirString.erase(projectDirString.size() - 2);// "\."����
		projectDirPath = projectDirString;
	}
	
	std::shared_ptr<library::Material> floorMaterial = std::make_shared<library::Material>(MaterialType::Phong);//�ٴ� �ؽ�ó
	std::filesystem::path floorTexturePath(L"Assets/Texture/seafloor.dds");//project dir�󿡼��� ���Path
	
	std::shared_ptr<library::Material> woodMaterial = std::make_shared<library::Material>(MaterialType::Phong);//���� �ؽ�ó
	std::filesystem::path woodTexturePath(L"Assets/Texture/wood.jpg");//project dir�󿡼��� ���Path
	
	std::shared_ptr<library::Material> mirrorMaterial = std::make_shared<library::Material>(MaterialType::Phong);//Texture���� Material
	
	std::shared_ptr<library::Material> ironPBRMaterial = std::make_shared<library::Material>(MaterialType::PBR);//PBR ö
	std::filesystem::path ironBaseColorTexturePath(L"Assets/Texture/IronPBRTexture/rustediron2_basecolor.png");//project dir�󿡼��� ���Path
	std::filesystem::path ironNormalTexturePath(L"Assets/Texture/IronPBRTexture/rustediron2_normal.png");//project dir�󿡼��� ���Path
	std::filesystem::path ironRoughnessTexturePath(L"Assets/Texture/IronPBRTexture/rustediron2_roughness.png");//project dir�󿡼��� ���Path
	std::filesystem::path ironMetallicTexturePath(L"Assets/Texture/IronPBRTexture/rustediron2_metallic.png");//project dir�󿡼��� ���Path

	std::shared_ptr<library::Material> pbrTestMaterials[7][7];
	for (UINT i = 0; i < 7u; i++)
	{
		for (UINT j = 0; j < 7u; j++)
		{//i�� 0�̸� roughness 0, j�� 0�̸� metallic 0
			pbrTestMaterials[i][j] = std::make_shared<library::Material>(MaterialType::PBR);
			pbrTestMaterials[i][j]->SetRoughness(i / 7.f);
			pbrTestMaterials[i][j]->SetMetallic(j / 7.f);
		}
	}



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
		AddMesh(cube1);
		AddMesh(cube2);
		AddMesh(cube3);
		AddMesh(cube4);
		AddMesh(cube5);
		AddMesh(plane);
		AddMesh(mirror);

		AddModel(cyborg);
		for (UINT i = 0; i < 7u; i++)
		{
			for (UINT j = 0; j < 7u; j++)
			{
				AddModel(pbrSpheres[i][j]);
			}
		}
		

		AddLight(light1);
		AddLight(light2);

		AddMaterial(floorMaterial);
		AddMaterial(woodMaterial);
		AddMaterial(mirrorMaterial);
		AddMaterial(ironPBRMaterial);
	}
	{//Mesh=>Material ���� ����
		cube1->SetMaterial(woodMaterial);
		cube2->SetMaterial(woodMaterial);
		cube3->SetMaterial(woodMaterial);
		cube4->SetMaterial(woodMaterial);
		cube5->SetMaterial(woodMaterial);
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
}
