#pragma once

#include "Scene\Scene.h"

/*
    https://benedikt-bitterli.me/resources/
*/
class BathroomScene : public library::Scene
{
public:
    BathroomScene();
    BathroomScene(const BathroomScene& other) = delete;
    BathroomScene(BathroomScene&& other) = delete;
    BathroomScene& operator=(const BathroomScene& other) = delete;
    BathroomScene& operator=(BathroomScene&& other) = delete;
    virtual ~BathroomScene() = default;
};