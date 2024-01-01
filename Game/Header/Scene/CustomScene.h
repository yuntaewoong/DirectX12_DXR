#pragma once

#include "Scene\Scene.h"

/*
    ���� ���� ��,�ؽ�ó ������ ��
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