#include "pch_game.h"
#include "Scene\CornellBoxScene.h"
#include <DirectXColors.h>

CornellBoxScene::CornellBoxScene()
	:
	Scene::Scene()
{
	std::shared_ptr<library::Model> cornellBox = std::make_shared<library::Model>(//�ڳڹڽ� ��
		L"Assets/Model/CornellBox/CornellBox-Sphere.obj",
		XMVectorSet(0.f, 2.f, -3.f, 1.0f),
		XMVectorSet(0.f, 0.f, 0.f, 1.0f),
		XMVectorSet(1.f, 1.f, 1.f, 1.f)
	);
	AddModel(cornellBox);

}
