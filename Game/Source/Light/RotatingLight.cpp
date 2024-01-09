#include "pch_game.h"
#include "Light\RotatingLight.h"

RotatingLight::RotatingLight(_In_ XMVECTOR position,_In_ FLOAT lumen) :
	PointLight(position,lumen)
{}

void RotatingLight::Update(_In_ FLOAT deltaTime)
{
    XMMATRIX rotate = XMMatrixRotationY(-2.0f * deltaTime);
    m_position = XMVector3Transform(m_position, rotate);
}