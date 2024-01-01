#pragma once

#include "Scene\Scene.h"

/*
    내가 직접 모델,텍스처 조작한 씬
*/
class CustomScene : public library::Scene
{
public:
    CustomScene();
    CustomScene(const CustomScene& other) = delete;
    CustomScene(CustomScene&& other) = delete;
    CustomScene& operator=(const CustomScene& other) = delete;
    CustomScene& operator=(CustomScene&& other) = delete;
    virtual ~CustomScene() = default;
};