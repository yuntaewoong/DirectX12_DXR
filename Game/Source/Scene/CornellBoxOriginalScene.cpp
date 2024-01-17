#include "pch_game.h"
#include "Scene\CornellBoxOriginalScene.h"
#include <DirectXColors.h>

CornellBoxOriginalScene::CornellBoxOriginalScene(
	_In_ XMVECTOR location,
    _In_ XMVECTOR rotation,
	_In_ XMVECTOR scale
)
	:
	Scene::Scene(location,rotation,scale)
{
	std::shared_ptr<library::Model> cornellBox = std::make_shared<library::Model>(//ÄÚ³Ú¹Ú½º ¸ðµ¨
		L"Assets/Model/CornellBox/cornell_box_multimaterial.obj",
		XMVectorSet(3.f, 0.f, 0.f, 1.0f),
		XMVectorSet(0.f, XM_PI, 0.f, 1.0f),
		XMVectorSet(0.01f, 0.01f, 0.01f, 1.f),
		TRUE
	);
	AddModel(cornellBox);

}
