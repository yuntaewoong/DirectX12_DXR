#include "pch_game.h"
#include "Scene\CornellBoxSphereScene.h"
#include <DirectXColors.h>

CornellBoxSphereScene::CornellBoxSphereScene(
	_In_ XMVECTOR location,
    _In_ XMVECTOR rotation,
	_In_ XMVECTOR scale
)
	:
	Scene::Scene(location,rotation,scale)
{
	std::shared_ptr<library::Model> cornellBox = std::make_shared<library::Model>(//�ڳڹڽ� ��
		L"Assets/Model/CornellBox/CornellBox-Sphere.obj",
		XMVectorSet(0.f, 2.f, -3.f, 1.0f),
		XMVectorSet(0.f, 0.f, 0.f, 1.0f),
		XMVectorSet(1.f, 1.f, 1.f, 1.f)
	);
	AddModel(cornellBox);

}
