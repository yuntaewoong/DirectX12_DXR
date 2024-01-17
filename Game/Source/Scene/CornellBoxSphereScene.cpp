#include "pch_game.h"
#include "Scene\CornellBoxSphereScene.h"
#include <DirectXColors.h>

CornellBoxSphereScene::CornellBoxSphereScene()
	:
	Scene::Scene()
{
	std::shared_ptr<library::Model> cornellBox = std::make_shared<library::Model>(//ÄÚ³Ú¹Ú½º ¸ðµ¨
		L"Assets/Model/CornellBox/CornellBox-Sphere.obj",
		XMVectorSet(0.f, 2.f, -3.f, 1.0f),
		XMVectorSet(0.f, 0.f, 0.f, 1.0f),
		XMVectorSet(1.f, 1.f, 1.f, 1.f)
	);
	AddModel(cornellBox);

}
