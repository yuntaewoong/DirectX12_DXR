#pragma once
#include "Light\PointLight.h"

class RotatingLight : public library::PointLight
{
public:
	RotatingLight(_In_ XMVECTOR position,_In_ FLOAT lumen);
	RotatingLight(const RotatingLight& other) = delete;
	RotatingLight(RotatingLight&& other) = delete;
	RotatingLight& operator=(const RotatingLight& other) = delete;
	RotatingLight& operator=(RotatingLight&& other) = delete;
	virtual ~RotatingLight() = default;
	virtual void Update(_In_ FLOAT deltaTime) override;
};