
#define HLSL
#include "../Include/HLSLCommon.hlsli"

[shader("miss")]
void MyShadowRayMissShader(inout ShadowRayPayload payload)
{
    payload.hit = 0.f; //�ȸ¾Ҵ�!(�׸���o)
}