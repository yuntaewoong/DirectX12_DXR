#pragma once

#include "Scene\Scene.h"

/*
    https://casual-effects.com/data/
*/
class CornellBoxOriginalScene : public library::Scene
{
public:
    CornellBoxOriginalScene(
        _In_ XMVECTOR location,
        _In_ XMVECTOR rotation,
        _In_ XMVECTOR scale
    );
    CornellBoxOriginalScene(const CornellBoxOriginalScene& other) = delete;
    CornellBoxOriginalScene(CornellBoxOriginalScene&& other) = delete;
    CornellBoxOriginalScene& operator=(const CornellBoxOriginalScene& other) = delete;
    CornellBoxOriginalScene& operator=(CornellBoxOriginalScene&& other) = delete;
    virtual ~CornellBoxOriginalScene() = default;
};