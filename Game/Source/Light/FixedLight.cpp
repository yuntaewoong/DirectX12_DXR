#include "pch_game.h"
#include "Light\FixedLight.h"

FixedLight::FixedLight(_In_ XMVECTOR position,_In_ FLOAT lumen) :
    PointLight(position,lumen)
{}

void FixedLight::Update(_In_ FLOAT deltaTime)
{}