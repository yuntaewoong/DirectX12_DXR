#include "pch_game.h"
#include "Scene\CornellBoxScene.h"
#include <DirectXColors.h>

CornellBoxScene::CornellBoxScene()
	:
	Scene::Scene()
{
	XMFLOAT4 color;
	XMStoreFloat4(&color, Colors::White);
	std::shared_ptr<library::Model> cornellBox = std::make_shared<library::Model>(//ÄÚ³Ú¹Ú½º ¸ðµ¨
		L"Assets/Model/CornellBox/CornellBox-Original.obj",
		XMVectorSet(0.f, 0.f, 0.f, 1.0f),
		XMVectorSet(0.f, 0.f, 0.f, 1.0f),
		XMVectorSet(1.f, 1.f, 1.f, 1.f),
		color
	);
	AddModel(cornellBox);
}
