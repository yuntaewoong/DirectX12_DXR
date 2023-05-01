#include "pch_game.h"
#include "Light\FixedLight.h"

FixedLight::FixedLight(_In_ XMVECTOR position) :
    PointLight(position)
{}

void FixedLight::Update(_In_ FLOAT deltaTime)
{}