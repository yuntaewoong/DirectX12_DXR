#define HLSL
#include "../../Include/HLSLCommon.hlsli"



/*================================================================================
    l_(variable_name) ===> local root signature�� ���ǵǴ� Resource(�� ������ Shader Table ���� �˸°� ����)
================================================================================*/
StructuredBuffer<Vertex> l_vertices : register(t2, space0);
ByteAddressBuffer l_indices : register(t3, space0);
ConstantBuffer<CubeConstantBuffer> l_cubeCB : register(b1);

//byteaddress buffer���� 3���� index�� �������� �Լ�
uint3 Load3x16BitIndices(uint offsetBytes)
{
    uint3 indices;
    
    //byte address buffer�� 32��Ʈ ������ �޸𸮸� load�Ҽ� ����
    //������ index�����ʹ� 2����Ʈ(16��Ʈ) * 3 = 6����Ʈ(48��Ʈ)��
    //�ε��� ������ 3���� ������� 64��Ʈ�� load�� �Ŀ� 48��Ʈ�� ���ϴ� ����� ���
    const uint dwordAlignedOffset = offsetBytes & ~3;
    const uint2 four16BitIndices = l_indices.Load2(dwordAlignedOffset);
 
    
    if (dwordAlignedOffset == offsetBytes)
    { //{ 0 1 | 2 - } ��
        indices.x = four16BitIndices.x & 0xffff;
        indices.y = (four16BitIndices.x >> 16) & 0xffff;
        indices.z = four16BitIndices.y & 0xffff;
    }
    else
    { //{ - 0 | 1 2 } ��
        indices.x = (four16BitIndices.x >> 16) & 0xffff;
        indices.y = four16BitIndices.y & 0xffff;
        indices.z = (four16BitIndices.y >> 16) & 0xffff;
    }

    return indices;
}
// hit������ world ��ǥ ����
float3 HitWorldPosition()
{
    return WorldRayOrigin() + RayTCurrent() * WorldRayDirection();
    //WorldRayOrigin() => hit�� Ray�� origin��ǥ
    //RayTCurrent() => hit�� Ray�� t��(tx+b)
    //WorldRayDirection() => hit�� Ray�� direction
}

//������ vertex��, hit������ barycentric���� �־��ָ� ���� ����� ����
float3 HitAttribute(float3 vertexAttribute[3], BuiltInTriangleIntersectionAttributes attr)
{
    return vertexAttribute[0] +
        attr.barycentrics.x * (vertexAttribute[1] - vertexAttribute[0]) +
        attr.barycentrics.y * (vertexAttribute[2] - vertexAttribute[0]);
}

// Diffuse���
float4 CalculateDiffuseLighting(float3 hitPosition, float3 normal)
{
    float3 pixelToLight = normalize(g_lightCB.position[0].xyz - hitPosition);
    float nDotL = max(0.0f, dot(pixelToLight, normal));
    return l_cubeCB.albedo * nDotL;
}
// Shadow Ray�� �̿��� �׸����̸� true, �ƴϸ� false����
bool IsInShadow(in float3 hitPosition, in float3 lightPosition)
{
    RayDesc rayDesc;
    rayDesc.Direction = normalize(lightPosition - hitPosition); //shadow ray ������ hit����->light����
    rayDesc.Origin = hitPosition;
    rayDesc.TMin = 0.001f;
    rayDesc.TMax = 10000.f;
    
    ShadowRayPayload shadowPayload;
    shadowPayload.hit = 1.f; //closest hit�� ������ �ʱ�ȭ
    
    TraceRay(
        g_scene, //acceleration structure
        RAY_FLAG_SKIP_CLOSEST_HIT_SHADER, //closest hit shader����(�ӵ� ���)(���߿� �ʿ��������� ���� ���ϰԵɵ�)
        TraceRayParameters::InstanceMask, //instance mask���� ���� x
        TraceRayParameters::HitGroupOffset[RayType::Shadow], //hit group index
        TraceRayParameters::GeometryStride, //shader table���� geometry�� ���� ����
        TraceRayParameters::MissShaderOffset[RayType::Shadow], //miss shader table���� ����� miss shader�� index
        rayDesc, //ray ����
        shadowPayload //payload
    );
    return shadowPayload.hit > 0.5f; //0.5f���� ũ�ٴ� �ǹ̴� miss shader�� ȣ����� �ʾƼ� �׸��� ������ �����Ѵٴ� �ǹ�
}

[shader("closesthit")]
void MyClosestHitShader(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    float3 hitPosition = HitWorldPosition();
    
    if (IsInShadow(hitPosition, g_lightCB.position[0].xyz))//shadow ray�� �̿��� �׸��� �˻�
    {
        payload.color = float4(0.1f, 0.1f, 0.1f, 1.f); //��ο� �׸���
        return;
    }
    
    uint indexSizeInBytes = 2; //index�� 16��Ʈ
    uint indicesPerTriangle = 3; //�ﰢ���� Vertex�� 3��
    uint triangleIndexStride = indicesPerTriangle * indexSizeInBytes; //primitive���� ���� �о���ϴ� Index����
    uint baseIndex = PrimitiveIndex() * triangleIndexStride; //�浹�� Primitive�� �ش��ϴ� index���� ù��°��
    
    const uint3 indices = Load3x16BitIndices(baseIndex); //base���� 3���� index�� �ε�6

    //index������ vertex normal�� ��������
    float3 vertexNormals[3] =
    {
        l_vertices[indices[0]].normal,
        l_vertices[indices[1]].normal,
        l_vertices[indices[2]].normal 
    };
 
    float3 triangleNormal = HitAttribute(vertexNormals, attr); //�����߽� ��ǥ��� normal�� �����ϱ�
    float4 diffuseColor = CalculateDiffuseLighting(hitPosition, triangleNormal);
    float4 color = float4(0.2f, 0.2f, 0.2f, 1.f) * l_cubeCB.albedo + diffuseColor;
    payload.color = color;

}