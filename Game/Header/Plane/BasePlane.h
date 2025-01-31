#pragma once
#include "Render\Mesh.h"

class BasePlane : public library::Mesh
{
public:
	BasePlane(
		_In_ XMVECTOR location,
		_In_ XMVECTOR rotation,
		_In_ XMVECTOR scale
	);
	BasePlane(const BasePlane& other) = delete;
	BasePlane(BasePlane&& other) = delete;
	BasePlane& operator=(const BasePlane& other) = delete;
	BasePlane& operator=(BasePlane&& other) = delete;
	virtual ~BasePlane() = default;

	void Update(_In_ FLOAT deltaTime) override;
private:
	static constexpr const Vertex VERTICES[] =
	{
		{.position = XMFLOAT3(-1.0f, 0.f, -1.0f), .uv = XMFLOAT2(1.0f, 0.0f), .normal = XMFLOAT3(0.0f, 1.0f, 0.0f),.tangent = XMFLOAT3(-1.f,0.f,0.f),.biTangent = XMFLOAT3(0.f,0.f,1.f) },
		{.position = XMFLOAT3(1.0f, 0.f, -1.0f), .uv = XMFLOAT2(0.0f, 0.0f), .normal = XMFLOAT3(0.0f, 1.0f, 0.0f),.tangent = XMFLOAT3(-1.f,0.f,0.f),.biTangent = XMFLOAT3(0.f,0.f,1.f) },
		{.position = XMFLOAT3(1.0f, 0.f,  1.0f), .uv = XMFLOAT2(0.0f, 1.0f), .normal = XMFLOAT3(0.0f, 1.0f, 0.0f),.tangent = XMFLOAT3(-1.f,0.f,0.f),.biTangent = XMFLOAT3(0.f,0.f,1.f) },
		{.position = XMFLOAT3(-1.0f, 0.f,  1.0f), .uv = XMFLOAT2(1.0f, 1.0f), .normal = XMFLOAT3(0.0f, 1.0f, 0.0f),.tangent = XMFLOAT3(-1.f,0.f,0.f),.biTangent = XMFLOAT3(0.f,0.f,1.f) }
	};
	static constexpr const Index INDICES[] =
	{
		{3}, {1}, {0},
		{2},{1},{3},
	};
};