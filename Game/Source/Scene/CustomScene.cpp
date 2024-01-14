#include "pch_game.h"
#include "Scene\CustomScene.h"
#include "Cube\BaseCube.h"
#include "Plane\BasePlane.h"
#include "Light\RotatingLight.h"
#include "Light\FixedLight.h"
#include "Material\Texture.h"
#include "Model\Model.h"
#include <DirectXColors.h>

CustomScene::CustomScene()
	:
	Scene::Scene()
{
	std::shared_ptr<library::Mesh> cube1 = std::make_shared<BaseCube>(//ť��1
		XMVectorSet(0.6f, 0.0f, 0.f, 1.0f),
		XMVectorSet(0.f, 0.f, 0.f, 1.0f),
		XMVectorSet(0.3f, 0.3f, 0.3f, 1.f)
	);

	std::shared_ptr<library::Mesh> cube2 = std::make_shared<BaseCube>(//ť��2
		XMVectorSet(-0.6f, 0.0f, 0.f, 1.0f),
		XMVectorSet(0.f, 0.f, 0.f, 1.0f),
		XMVectorSet(0.3f, 0.3f, 0.3f, 1.f)
	);

	std::shared_ptr<library::Mesh> cube3 = std::make_shared<BaseCube>(//ť��3
		XMVectorSet(0.0f, 0.6f, 0.f, 1.0f),
		XMVectorSet(0.f, 0.f, 0.f, 1.0f),
		XMVectorSet(0.3f, 0.3f, 0.3f, 1.f)
	);

	std::shared_ptr<library::Mesh> cube4 = std::make_shared<BaseCube>(//ť��4
		XMVectorSet(0.0f, 0.0f, 0.6f, 1.0f),
		XMVectorSet(0.f, 0.f, 0.f, 1.0f),
		XMVectorSet(0.3f, 0.3f, 0.3f, 1.f)
	);

	std::shared_ptr<library::Mesh> cube5 = std::make_shared<BaseCube>(//ť��5
		XMVectorSet(0.0f, 0.0f, -0.6f, 1.0f),
		XMVectorSet(0.f, 0.f, 0.f, 1.0f),
		XMVectorSet(0.3f, 0.3f, 0.3f, 1.f)
	);

	std::shared_ptr<library::Mesh> plane = std::make_shared<BasePlane>(//�ٴ�
		XMVectorSet(0.f, -0.3f, 0.f, 1.0f),
		XMVectorSet(0.f, 0.f, 0.f, 1.0f),
		XMVectorSet(15.f, 1.f, 15.f, 1.f)
	);

	std::shared_ptr<library::Mesh> mirror = std::make_shared<BasePlane>(//�ſ�
		XMVectorSet(0.f, 0.6f, 2.5f, 1.0f),
		XMVectorSet(0.f, XM_PIDIV2, -XM_PIDIV2, 1.0f),
		XMVectorSet(15.f, 1.f, 15.f, 1.f)
	);

	std::shared_ptr<library::Model> cyborg = std::make_shared<library::Model>(//���̺��� ��
		L"Assets/Model/cyborg/cyborg.obj",
		XMVectorSet(-3.f, 1.f, 0.f, 1.0f),
		XMVectorSet(0.f, 0.f, 0.f, 1.0f),
		XMVectorSet(1.f, 1.f, 1.f, 1.f)
	);

	std::shared_ptr<library::Model> pbrSpheres[7][7];
	for (UINT i = 0; i < 7u; i++)
	{
		for (UINT j = 0; j < 7u; j++)
		{
			pbrSpheres[i][j] = std::make_shared<library::Model>(
				L"Assets/Model/sphere/source/sphere.fbx",
				XMVectorSet(1.f + static_cast<float>(i)/1.4f, 1.f + static_cast<float>(j) /1.4f, 1.f, 1.0f),
				XMVectorSet(0.f, 0.f, 0.f, 1.0f),
				XMVectorSet(0.3f, 0.3f, 0.3f, 1.0f)
			);
		}
	}
	
	


	std::shared_ptr<library::PointLight> light1 = std::make_shared<RotatingLight>(XMVectorSet(0.f, 5.f, -5.f,1.f),200.0f);
	std::shared_ptr<library::PointLight> light2 = std::make_shared<FixedLight>(XMVectorSet(3.f, 3.f, -5.f, 1.f),300.0f);


	std::filesystem::path projectDirPath;
	{
		std::string projectDirString = EXPAND(PROJECT_DIR);//project_dir�� ���ڿ�ȭ
		projectDirString.erase(0, 1);//Root����
		projectDirString.erase(projectDirString.size() - 2);// "\."����
		projectDirPath = projectDirString;
	}
	std::shared_ptr<library::Material> emissiveMaterial = std::make_shared<library::Material>(XMFLOAT4(1.f,1.f,1.f,1.f));
	emissiveMaterial->SetEmission(150.0f);

	std::shared_ptr<library::Material> floorMaterial = std::make_shared<library::Material>();//�ٴ� �ؽ�ó
	std::filesystem::path floorTexturePath(L"Assets/Texture/seafloor.dds");//project dir�󿡼��� ���Path
	
	std::shared_ptr<library::Material> woodMaterial = std::make_shared<library::Material>();//���� �ؽ�ó
	std::filesystem::path woodTexturePath(L"Assets/Texture/wood.jpg");//project dir�󿡼��� ���Path
	
	std::shared_ptr<library::Material> mirrorMaterial = std::make_shared<library::Material>(XMFLOAT4(1.f,1.f,1.f,1.f));//Texture���� Material
	mirrorMaterial->SetMetallic(0.8f);

	std::shared_ptr<library::Material> ironPBRMaterial = std::make_shared<library::Material>();//PBR ö
	std::filesystem::path ironBaseColorTexturePath(L"Assets/Texture/IronPBRTexture/rustediron2_basecolor.png");//project dir�󿡼��� ���Path
	std::filesystem::path ironNormalTexturePath(L"Assets/Texture/IronPBRTexture/rustediron2_normal.png");//project dir�󿡼��� ���Path
	std::filesystem::path ironRoughnessTexturePath(L"Assets/Texture/IronPBRTexture/rustediron2_roughness.png");//project dir�󿡼��� ���Path
	std::filesystem::path ironMetallicTexturePath(L"Assets/Texture/IronPBRTexture/rustediron2_metallic.png");//project dir�󿡼��� ���Path

	std::shared_ptr<library::Material> pbrTestMaterials[7][7];
	for (UINT i = 0; i < 7u; i++)
	{
		for (UINT j = 0; j < 7u; j++)
		{//i�� 0�̸� roughness 0, j�� 0�̸� metallic 0
			pbrTestMaterials[i][j] = std::make_shared<library::Material>(XMFLOAT4(1.0f,0.0f,0.0f,0.0f));
			pbrTestMaterials[i][j]->SetRoughness(i / 7.f);
			//pbrTestMaterials[i][j]->SetMetallic(j / 7.f);
			pbrTestMaterials[i][j]->SetMetallic(1.0f);
		}
	}



	{//Material=>Texture���� ����
		floorMaterial->SetAlbedoTexture(std::make_shared<library::Texture>(projectDirPath / floorTexturePath));
		woodMaterial->SetAlbedoTexture(std::make_shared<library::Texture>(projectDirPath / woodTexturePath));
		ironPBRMaterial->SetAlbedoTexture(std::make_shared<library::Texture>(projectDirPath / ironBaseColorTexturePath));
		ironPBRMaterial->SetNormalTexture(std::make_shared<library::Texture>(projectDirPath / ironNormalTexturePath));
		ironPBRMaterial->SetRoughnessTexture(std::make_shared<library::Texture>(projectDirPath / ironRoughnessTexturePath));
		ironPBRMaterial->SetMetallicTexture(std::make_shared<library::Texture>(projectDirPath / ironMetallicTexturePath));
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

		AddMaterial(emissiveMaterial);
		AddMaterial(woodMaterial);
		AddMaterial(floorMaterial);
		AddMaterial(mirrorMaterial);
		AddMaterial(ironPBRMaterial);
	}
	{//Mesh=>Material ���� ����
		cube1->SetMaterial(emissiveMaterial/*woodMaterial*/);
		cube2->SetMaterial(ironPBRMaterial);
		cube3->SetMaterial(ironPBRMaterial);
		cube4->SetMaterial(emissiveMaterial/*woodMaterial*/);
		cube5->SetMaterial(emissiveMaterial);
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
