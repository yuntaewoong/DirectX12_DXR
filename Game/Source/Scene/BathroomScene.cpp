#include "pch_game.h"
#include "Scene\BathroomScene.h"
#include <DirectXColors.h>

BathroomScene::BathroomScene()
	:
	Scene::Scene()
{
	XMFLOAT4 color;
	XMStoreFloat4(&color, Colors::White);
	std::shared_ptr<library::Model> bathroom = std::make_shared<library::Model>(//bathroom ¸ðµ¨
		L"Assets/Model/bathroom/scene-v4.pbrt",
		XMVectorSet(0.f, 0.f, 0.f, 1.0f),
		XMVectorSet(0.f, 0.f, 0.f, 1.0f),
		XMVectorSet(1.f, 1.f, 1.f, 1.f),
		color
	);
	AddModel(bathroom);
}
