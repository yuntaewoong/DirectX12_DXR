#pragma once

#include "Scene\Scene.h"

/*
    https://casual-effects.com/data/
*/
class CornellBoxScene : public library::Scene
{
public:
    CornellBoxScene();
    CornellBoxScene(const CornellBoxScene& other) = delete;
    CornellBoxScene(CornellBoxScene&& other) = delete;
    CornellBoxScene& operator=(const CornellBoxScene& other) = delete;
    CornellBoxScene& operator=(CornellBoxScene&& other) = delete;
    virtual ~CornellBoxScene() = default;
};