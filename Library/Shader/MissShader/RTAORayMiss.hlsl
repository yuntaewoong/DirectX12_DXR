#define HLSL
#include "../Include/HLSLCommon.hlsli"

[shader("miss")]
void RTAORayMissShader(inout RTAORayPayload payload)
{
    payload.tHit = RTAO_RADIUS;//������ ������ hit���� �ʾ����Ƿ� �������� payload�� ����
}