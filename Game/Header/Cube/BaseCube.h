#pragma once
#include "Render\Mesh.h"

class BaseCube : public library::Mesh
{
public:
	BaseCube(
		_In_ XMVECTOR location,
		_In_ XMVECTOR rotation,
		_In_ XMVECTOR scale
	);
	BaseCube(const BaseCube& other) = delete;
	BaseCube(BaseCube&& other) = delete;
	BaseCube& operator=(const BaseCube& other) = delete;
	BaseCube& operator=(BaseCube&& other) = delete;
	~BaseCube() = default;

	void Update(_In_ FLOAT deltaTime) override;
private:
	static constexpr const Vertex VERTICES[] =
	{
		{.position = XMFLOAT3(-1.0f, 1.0f, -1.0f), .uv = XMFLOAT2(1.0f, 0.0f), .normal = XMFLOAT3(0.0f, 1.0f, 0.0f), .tangent = XMFLOAT3(-1.f,0.f,0.f),.biTangent = XMFLOAT3(0.f,0.f,1.f)},
		{.position = XMFLOAT3(1.0f, 1.0f, -1.0f), .uv = XMFLOAT2(0.0f, 0.0f), .normal = XMFLOAT3(0.0f, 1.0f, 0.0f) ,.tangent = XMFLOAT3(-1.f,0.f,0.f),.biTangent = XMFLOAT3(0.f,0.f,1.f)},
		{.position = XMFLOAT3(1.0f, 1.0f,  1.0f), .uv = XMFLOAT2(0.0f, 1.0f), .normal = XMFLOAT3(0.0f, 1.0f, 0.0f) ,.tangent = XMFLOAT3(-1.f,0.f,0.f),.biTangent = XMFLOAT3(0.f,0.f,1.f)},
		{.position = XMFLOAT3(-1.0f, 1.0f,  1.0f), .uv = XMFLOAT2(1.0f, 1.0f), .normal = XMFLOAT3(0.0f, 1.0f, 0.0f),.tangent = XMFLOAT3(-1.f,0.f,0.f),.biTangent = XMFLOAT3(0.f,0.f,1.f)},

		{.position = XMFLOAT3(-1.0f, -1.0f, -1.0f), .uv = XMFLOAT2(0.0f, 0.0f), .normal = XMFLOAT3(0.0f, -1.0f, 0.0f),.tangent = XMFLOAT3(1.f,0.f,0.f),.biTangent = XMFLOAT3(0.f,0.f,1.f) },
		{.position = XMFLOAT3(1.0f, -1.0f, -1.0f), .uv = XMFLOAT2(1.0f, 0.0f), .normal = XMFLOAT3(0.0f, -1.0f, 0.0f) ,.tangent = XMFLOAT3(1.f,0.f,0.f),.biTangent = XMFLOAT3(0.f,0.f,1.f) },
		{.position = XMFLOAT3(1.0f, -1.0f,  1.0f), .uv = XMFLOAT2(1.0f, 1.0f), .normal = XMFLOAT3(0.0f, -1.0f, 0.0f) ,.tangent = XMFLOAT3(1.f,0.f,0.f),.biTangent = XMFLOAT3(0.f,0.f,1.f) },
		{.position = XMFLOAT3(-1.0f, -1.0f,  1.0f), .uv = XMFLOAT2(0.0f, 1.0f), .normal = XMFLOAT3(0.0f, -1.0f, 0.0f) ,.tangent = XMFLOAT3(1.f,0.f,0.f),.biTangent = XMFLOAT3(0.f,0.f,1.f)},

		{.position = XMFLOAT3(-1.0f, -1.0f,  1.0f), .uv = XMFLOAT2(0.0f, 1.0f), .normal = XMFLOAT3(-1.0f, 0.0f, 0.0f),.tangent = XMFLOAT3(0.f,0.f,-1.f),.biTangent = XMFLOAT3(0.f,-1.f,0.f)},
		{.position = XMFLOAT3(-1.0f, -1.0f, -1.0f), .uv = XMFLOAT2(1.0f, 1.0f), .normal = XMFLOAT3(-1.0f, 0.0f, 0.0f),.tangent = XMFLOAT3(0.f,0.f,-1.f),.biTangent = XMFLOAT3(0.f,-1.f,0.f)},
		{.position = XMFLOAT3(-1.0f,  1.0f, -1.0f), .uv = XMFLOAT2(1.0f, 0.0f), .normal = XMFLOAT3(-1.0f, 0.0f, 0.0f),.tangent = XMFLOAT3(0.f,0.f,-1.f),.biTangent = XMFLOAT3(0.f,-1.f,0.f)},
		{.position = XMFLOAT3(-1.0f,  1.0f,  1.0f), .uv = XMFLOAT2(0.0f, 0.0f), .normal = XMFLOAT3(-1.0f, 0.0f, 0.0f),.tangent = XMFLOAT3(0.f,0.f,-1.f),.biTangent = XMFLOAT3(0.f,-1.f,0.f)},

		{.position = XMFLOAT3(1.0f, -1.0f,  1.0f), .uv = XMFLOAT2(1.0f, 1.0f), .normal = XMFLOAT3(1.0f, 0.0f, 0.0f),.tangent = XMFLOAT3(0.f,0.f,1.f),.biTangent = XMFLOAT3(0.f,-1.f,0.f) },
		{.position = XMFLOAT3(1.0f, -1.0f, -1.0f), .uv = XMFLOAT2(0.0f, 1.0f), .normal = XMFLOAT3(1.0f, 0.0f, 0.0f),.tangent = XMFLOAT3(0.f,0.f,1.f),.biTangent = XMFLOAT3(0.f,-1.f,0.f) },
		{.position = XMFLOAT3(1.0f,  1.0f, -1.0f), .uv = XMFLOAT2(0.0f, 0.0f), .normal = XMFLOAT3(1.0f, 0.0f, 0.0f),.tangent = XMFLOAT3(0.f,0.f,1.f),.biTangent = XMFLOAT3(0.f,-1.f,0.f)},
		{.position = XMFLOAT3(1.0f,  1.0f,  1.0f), .uv = XMFLOAT2(1.0f, 0.0f), .normal = XMFLOAT3(1.0f, 0.0f, 0.0f),.tangent = XMFLOAT3(0.f,0.f,1.f),.biTangent = XMFLOAT3(0.f,-1.f,0.f) },

		{.position = XMFLOAT3(-1.0f, -1.0f, -1.0f), .uv = XMFLOAT2(0.0f, 1.0f), .normal = XMFLOAT3(0.0f, 0.0f, -1.0f),.tangent = XMFLOAT3(1.f,0.f,0.f),.biTangent = XMFLOAT3(0.f,-1.f,0.f)},
		{.position = XMFLOAT3(1.0f, -1.0f, -1.0f), .uv = XMFLOAT2(1.0f, 1.0f), .normal = XMFLOAT3(0.0f, 0.0f, -1.0f),.tangent = XMFLOAT3(1.f,0.f,0.f),.biTangent = XMFLOAT3(0.f,-1.f,0.f) },
		{.position = XMFLOAT3(1.0f,  1.0f, -1.0f), .uv = XMFLOAT2(1.0f, 0.0f), .normal = XMFLOAT3(0.0f, 0.0f, -1.0f),.tangent = XMFLOAT3(1.f,0.f,0.f),.biTangent = XMFLOAT3(0.f,-1.f,0.f) },
		{.position = XMFLOAT3(-1.0f,  1.0f, -1.0f), .uv = XMFLOAT2(0.0f, 0.0f), .normal = XMFLOAT3(0.0f, 0.0f, -1.0f),.tangent = XMFLOAT3(1.f,0.f,0.f),.biTangent = XMFLOAT3(0.f,-1.f,0.f) },

		{.position = XMFLOAT3(-1.0f, -1.0f, 1.0f), .uv = XMFLOAT2(1.0f, 1.0f), .normal = XMFLOAT3(0.0f, 0.0f, 1.0f),.tangent = XMFLOAT3(-1.f,0.f,0.f),.biTangent = XMFLOAT3(0.f,-1.f,0.f) },
		{.position = XMFLOAT3(1.0f, -1.0f, 1.0f), .uv = XMFLOAT2(0.0f, 1.0f), .normal = XMFLOAT3(0.0f, 0.0f, 1.0f),.tangent = XMFLOAT3(-1.f,0.f,0.f),.biTangent = XMFLOAT3(0.f,-1.f,0.f)  },
		{.position = XMFLOAT3(1.0f,  1.0f, 1.0f), .uv = XMFLOAT2(0.0f, 0.0f), .normal = XMFLOAT3(0.0f, 0.0f, 1.0f),.tangent = XMFLOAT3(-1.f,0.f,0.f),.biTangent = XMFLOAT3(0.f,-1.f,0.f)  },
		{.position = XMFLOAT3(-1.0f,  1.0f, 1.0f), .uv = XMFLOAT2(1.0f, 0.0f), .normal = XMFLOAT3(0.0f, 0.0f, 1.0f),.tangent = XMFLOAT3(-1.f,0.f,0.f),.biTangent = XMFLOAT3(0.f,-1.f,0.f)  },
	};
	static constexpr const Index INDICES[] =
	{
		{3}, {1}, {0},
		{2},{1},{3},

		{6},{4},{5},
		{7},{4},{6},

		{11},{9},{8},
		{10},{9},{11},

		{14},{12},{13},
		{15},{12},{14},

		{19},{17},{16},
		{18},{17},{19},

		{22},{20},{21},
		{23},{20},{22}
	};
};