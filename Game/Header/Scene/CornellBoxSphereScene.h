#pragma once

#include "Scene\Scene.h"

/*
    https://casual-effects.com/data/
*/
class CornellBoxSphereScene : public library::Scene
{
public:
    CornellBoxSphereScene(
        _In_ XMVECTOR location,
        _In_ XMVECTOR rotation,
        _In_ XMVECTOR scale
    );
    CornellBoxSphereScene(const CornellBoxSphereScene& other) = delete;
    CornellBoxSphereScene(CornellBoxSphereScene&& other) = delete;
    CornellBoxSphereScene& operator=(const CornellBoxSphereScene& other) = delete;
    CornellBoxSphereScene& operator=(CornellBoxSphereScene&& other) = delete;
    virtual ~CornellBoxSphereScene() = default;
};