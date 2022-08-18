#define HLSL
#include "../../Include/HLSLCommon.hlsli"



/*================================================================================
    l_(variable_name) ===> local root signature�� ���ǵǴ� Resource(�� ������ Shader Table ���� �˸°� ����)
================================================================================*/
//SRV
StructuredBuffer<Vertex> l_vertices : register(t2, space0);
ByteAddressBuffer l_indices : register(t3, space0);
Texture2D l_diffuseTexture : register(t4);
//CBV
ConstantBuffer<RenderableConstantBuffer> l_renderableCB : register(b1);
//Static Sampler
SamplerState l_sampler : register(s0,space0);

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

//������ vertex��, hit������ barycentric���� �־��ָ� ���� ����� ����(Float3����)
float3 HitAttributeFloat3(float3 vertexAttribute[3], BuiltInTriangleIntersectionAttributes attr)
{
    return vertexAttribute[0] +
        attr.barycentrics.x * (vertexAttribute[1] - vertexAttribute[0]) +
        attr.barycentrics.y * (vertexAttribute[2] - vertexAttribute[0]);
}
//������ vertex��, hit������ barycentric���� �־��ָ� ���� ����� ����(Float2����)
float2 HitAttributeFloat2(float2 vertexAttribute[3], BuiltInTriangleIntersectionAttributes attr)
{
    return vertexAttribute[0] +
        attr.barycentrics.x * (vertexAttribute[1] - vertexAttribute[0]) +
        attr.barycentrics.y * (vertexAttribute[2] - vertexAttribute[0]);
}

// Diffuse���
float3 CalculateDiffuseLighting(float3 hitPosition, float3 normal,float2 uv)
{
    float3 pixelToLight = normalize(g_lightCB.position[0].xyz - hitPosition);
    float3 nDotL = max(0.0f, dot(pixelToLight, normal));
    float3 diffuseTexelColor = float3(1.f, 1.f, 1.f);
    if(l_renderableCB.hasTexture == 1)
    {
        diffuseTexelColor = l_diffuseTexture.SampleLevel(l_sampler, uv, 0).xyz; //Shadel Model lib 6_3������ Sample�Լ� �����Ͽ�����       
    }
    return l_renderableCB.albedo * nDotL * diffuseTexelColor;
}

// Specullar���
float3 CalculateSpecullarLighting(float3 hitPosition, float3 normal, float2 uv)
{
    float3 lightToHit = normalize(hitPosition - g_lightCB.position[0].xyz);
    float3 cameraToHit = normalize(hitPosition - g_cameraCB.cameraPosition);
    float3 reflectDirection = normalize(reflect(lightToHit, normal));
    
    return pow(max(dot(-cameraToHit, reflectDirection), 0.0f), 15.0f) * l_renderableCB.albedo;
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
    bool isInShadow = false;
    
    if (IsInShadow(hitPosition, g_lightCB.position[0].xyz))//shadow ray�� �̿��� �׸��� �˻�
    {
        isInShadow = true;
    }
    
    uint indexSizeInBytes = 2; //index�� 16��Ʈ
    uint indicesPerTriangle = 3; //�ﰢ���� Vertex�� 3��
    uint triangleIndexStride = indicesPerTriangle * indexSizeInBytes; //primitive���� ���� �о���ϴ� Index����
    uint baseIndex = PrimitiveIndex() * triangleIndexStride; //�浹�� Primitive�� �ش��ϴ� index���� ù��°��
    
    const uint3 indices = Load3x16BitIndices(baseIndex); //base���� 3���� index�� �ε�6

    float3 vertexNormals[3] =
    { //index������ world space vertex normal�� ��������
        normalize(mul(float4(l_vertices[indices[0]].normal, 0), l_renderableCB.world).xyz),
        normalize(mul(float4(l_vertices[indices[1]].normal, 0), l_renderableCB.world).xyz),
        normalize(mul(float4(l_vertices[indices[2]].normal, 0), l_renderableCB.world).xyz)
    };
    float2 vertexUV[3] =
    { //index������ vertex uv�� ��������
        l_vertices[indices[0]].uv,
        l_vertices[indices[1]].uv,
        l_vertices[indices[2]].uv 
    };
    
    
    float3 triangleNormal = HitAttributeFloat3(vertexNormals, attr); //�����߽� ��ǥ��� normal�� �����ϱ�
    float2 triangleUV = HitAttributeFloat2(vertexUV, attr);//�����߽� ��ǥ��� UV�� �����ϱ�
    float3 ambientColor = float3(0.2f, 0.2f, 0.2f);
    if(l_renderableCB.hasTexture == 1)
    {
        ambientColor = ambientColor * l_diffuseTexture.SampleLevel(l_sampler, triangleUV, 0).xyz;
    }
    float3 diffuseColor = CalculateDiffuseLighting(hitPosition, triangleNormal,triangleUV);
    float3 specullarColor = CalculateSpecullarLighting(hitPosition, triangleNormal, triangleUV);
    if(isInShadow)
    {
        diffuseColor = diffuseColor * float3(0.1f, 0.1f, 0.1f);
        specullarColor = specullarColor * float3(0.1f, 0.1f, 0.1f);
    }
    
    float3 color = ambientColor + diffuseColor+ specullarColor;
    payload.color = float4(color, 1);
    
    

}