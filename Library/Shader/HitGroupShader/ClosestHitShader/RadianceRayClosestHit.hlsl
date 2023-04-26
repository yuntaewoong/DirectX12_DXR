#define HLSL
#include "../../Include/HLSLCommon.hlsli"
#include "../../Include/RadianceRayTrace.hlsli"
#include "../../Include/ShadowRayTrace.hlsli"
#include "../../Include/RTAORayTrace.hlsli"
#include "../../Include/BxDF/BRDF/PhongModel.hlsli"

/*================================================================================
    l_(variable_name) ===> local root signature�� ���ǵǴ� Resource(�� ������ Shader Table ���� �˸°� ����)
================================================================================*/
//SRV
StructuredBuffer<Vertex> l_vertices : register(t2, space0);
ByteAddressBuffer l_indices : register(t3, space0);
Texture2D l_diffuseTexture : register(t4);
Texture2D l_normalTexture : register(t5);
Texture2D l_specularTexture : register(t6);
Texture2D l_roughnessTexture : register(t7);
Texture2D l_metallicTexture : register(t8);

//CBV
ConstantBuffer<MeshConstantBuffer> l_meshCB : register(b1);
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

//�븻���� ����� ���ο� �븻�� ����
float3 CalculateNormalmapNormal(float3 originNormal,float3 tangent,float3 biTangent,float2 uv)
{
    if(!l_meshCB.hasNormalTexture)
        return originNormal;
    float3 newNormal = l_normalTexture.SampleLevel(l_sampler, uv, 0).xyz;
    newNormal = (newNormal * 2.0f) - 1.0f;
    newNormal = (newNormal.x * tangent) + (newNormal.y * biTangent) + (newNormal.z * originNormal);//TBN��ȯ
    return normalize(newNormal);
}
[shader("closesthit")]
void MyClosestHitShader(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    float3 hitPosition = HitWorldPosition();
    uint indexSizeInBytes = 2; //index�� 16��Ʈ
    uint indicesPerTriangle = 3; //�ﰢ���� Vertex�� 3��
    uint triangleIndexStride = indicesPerTriangle * indexSizeInBytes; //primitive���� ���� �о���ϴ� Index����
    uint baseIndex = PrimitiveIndex() * triangleIndexStride; //�浹�� Primitive�� �ش��ϴ� index���� ù��°��
    
    const uint3 indices = Load3x16BitIndices(baseIndex); //base���� 3���� index�� �ε�6

    float3 vertexNormals[3] =
    { //index������ world space vertex normal�� ��������
        normalize(mul(float4(l_vertices[indices[0]].normal, 0), l_meshCB.world).xyz),
        normalize(mul(float4(l_vertices[indices[1]].normal, 0), l_meshCB.world).xyz),
        normalize(mul(float4(l_vertices[indices[2]].normal, 0), l_meshCB.world).xyz)
    };
    float3 vertexTangent[3] =
    { //index������ world space vertex tangent�� ��������
        normalize(mul(float4(l_vertices[indices[0]].tangent, 0), l_meshCB.world).xyz),
        normalize(mul(float4(l_vertices[indices[1]].tangent, 0), l_meshCB.world).xyz),
        normalize(mul(float4(l_vertices[indices[2]].tangent, 0), l_meshCB.world).xyz)
    };
    float3 vertexBiTangent[3] =
    { //index������ world space vertex biTangent�� ��������
        normalize(mul(float4(l_vertices[indices[0]].biTangent, 0), l_meshCB.world).xyz),
        normalize(mul(float4(l_vertices[indices[1]].biTangent, 0), l_meshCB.world).xyz),
        normalize(mul(float4(l_vertices[indices[2]].biTangent, 0), l_meshCB.world).xyz)
    };
    float2 vertexUV[3] =
    { //index������ vertex uv�� ��������
        l_vertices[indices[0]].uv,
        l_vertices[indices[1]].uv,
        l_vertices[indices[2]].uv 
    };
    
    float3 triangleNormal = HitAttributeFloat3(vertexNormals, attr); //�����߽� ��ǥ��� normal�� �����ϱ�
    float3 triangleTangent = HitAttributeFloat3(vertexTangent, attr); //�����߽� ��ǥ��� tangent�� �����ϱ�
    float3 triangleBitangent = HitAttributeFloat3(vertexBiTangent, attr); //�����߽� ��ǥ��� biTangent�� �����ϱ�
    float2 triangleUV = HitAttributeFloat2(vertexUV, attr); //�����߽� ��ǥ��� UV�� �����ϱ�
    triangleNormal = CalculateNormalmapNormal(triangleNormal, triangleTangent, triangleBitangent, triangleUV); //�븻���� �ִٸ� �븻������
    
    {//PhongShading��
        float3 ambientColor = l_meshCB.albedo.rgb * float3(0.2f, 0.2f, 0.2f);
        {//RTAO(Ray Tracing Ambient Occlusion ���)
            ambientColor = TraceRTAORay(hitPosition, triangleNormal, payload.recursionDepth);
            ambientColor /= 3.f; //�ʹ� �����ʰ� ����
        }
        float3 diffuseColor = l_meshCB.albedo.rgb;
        float3 specularColor = float3(0.f, 0.f, 0.f);
        if (l_meshCB.hasDiffuseTexture)
        {// ��ǻ����� �����Ѵٸ� ���
            ambientColor = ambientColor * l_diffuseTexture.SampleLevel(l_sampler, triangleUV, 0).rgb;
            diffuseColor = l_diffuseTexture.SampleLevel(l_sampler, triangleUV, 0).rgb;
        }
        if (l_meshCB.hasSpecularTexture)
        { //����ŧ�� ���� �����Ѵٸ� ���
            specularColor = l_specularTexture.SampleLevel(l_sampler, triangleUV, 0).xyz;
        }
        float3 pointToLight = normalize(g_lightCB.position[0].xyz - hitPosition);
        float3 pointToCamera = normalize(g_cameraCB.cameraPosition.xyz - hitPosition);
        float3 lightColor = float3(1.f, 1.f, 1.f);
        float3 lightDistance = g_lightCB.position[0].xyz - hitPosition;
        float lightAttenuation = 1.0 / (1.0f + 0.09f * lightDistance + 0.032f * (lightDistance * lightDistance));
        bool isInShadow = TraceShadowRay(hitPosition, g_lightCB.position[0].xyz, payload.recursionDepth);
        float3 color = BxDF::PhongShade(
            ambientColor,
            diffuseColor,
            specularColor,
            triangleNormal,
            pointToLight,
            pointToCamera,
            lightColor,
            lightAttenuation,
            isInShadow
        );
        float3 reflectedColor = float3(0.f, 0.f, 0.f);
        if(l_meshCB.reflectivity > 0.0f)
        {
            float3 nextRayDirection = reflect(WorldRayDirection(), triangleNormal); //�ݻ������ Trace�� ���� Ray�� Direction
            reflectedColor = TraceRadianceRay(hitPosition, nextRayDirection, payload.recursionDepth).rgb;
        }
        payload.color = float4(color * (1.0f - l_meshCB.reflectivity) + reflectedColor * l_meshCB.reflectivity, 1);
    }
}