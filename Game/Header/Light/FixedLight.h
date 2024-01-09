#pragma once
#include "Light\PointLight.h"

class FixedLight : public library::PointLight
{
public:
	FixedLight(_In_ XMVECTOR position,_In_ FLOAT lumen);
	FixedLight(const FixedLight& other) = delete;
	FixedLight(FixedLight&& other) = delete;
	FixedLight& operator=(const FixedLight& other) = delete;
	FixedLight& operator=(FixedLight&& other) = delete;
	virtual ~FixedLight() = default;
	virtual void Update(_In_ FLOAT deltaTime) override;
};