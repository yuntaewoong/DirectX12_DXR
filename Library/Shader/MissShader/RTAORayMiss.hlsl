#define HLSL
#include "../Include/HLSLCommon.hlsli"

[shader("miss")]
void RTAORayMissShader(inout RTAORayPayload payload)
{
    payload.tHit = RTAO_RADIUS;//반지름 끝까지 hit하지 않았으므로 반지름값 payload에 저장
}