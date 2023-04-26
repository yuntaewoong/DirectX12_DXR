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

// Diffuse���
float3 CalculateDiffuseLighting(float3 hitPosition, float3 normal,float2 uv)
{
    float3 pixelToLight = normalize(g_lightCB.position[0].xyz - hitPosition);
    float3 nDotL = max(0.0f, dot(pixelToLight, normal));
    float3 diffuseTexelColor = float3(1.f, 1.f, 1.f);
    if(l_meshCB.hasDiffuseTexture == 1)
    {
        diffuseTexelColor = l_diffuseTexture.SampleLevel(l_sampler, uv, 0).xyz; //Shadel Model lib 6_3������ Sample�Լ� �����Ͽ�����       
    }
    return l_meshCB.albedo.xyz * nDotL * diffuseTexelColor;
}

// Specullar���
float3 CalculateSpecullarLighting(float3 hitPosition, float3 normal,float2 uv)
{
    float3 specularSample = float3(1.f, 1.f, 1.f);
    if(l_meshCB.hasSpecularTexture)
    {//����ŧ�� ���� �����Ѵٸ� ���
        specularSample = l_specularTexture.SampleLevel(l_sampler, uv, 0).xyz;
    }
    float3 lightToHit = normalize(hitPosition - g_lightCB.position[0].xyz);
    float3 cameraToHit = normalize(hitPosition - g_cameraCB.cameraPosition.xyz);
    float3 reflectDirection = normalize(reflect(lightToHit, normal));
    
    return specularSample * pow(max(dot(-cameraToHit, reflectDirection), 0.0f), 15.0f) * l_meshCB.albedo.xyz;
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
    
    float3 ambientColor = float3(0.2f, 0.2f, 0.2f);
    float3 diffuseColor = float3(0.f, 0.f, 0.f);
    float3 specullarColor = float3(0.f, 0.f, 0.f);
    float3 reflectedColor = float3(0.f, 0.f, 0.f);
    {//RTAO(Ray Tracing Ambient Occlusion ���)
        ambientColor = TraceRTAORay(hitPosition, triangleNormal, payload.recursionDepth);
        ambientColor /= 3.f; //�ʹ� �����ʰ� ����
    }
    {//Phong Shading���
        if (l_meshCB.hasDiffuseTexture == 1)
        {
            ambientColor = ambientColor * l_diffuseTexture.SampleLevel(l_sampler, triangleUV, 0).xyz;
        }
        diffuseColor = CalculateDiffuseLighting(hitPosition, triangleNormal, triangleUV);
        specullarColor = CalculateSpecullarLighting(hitPosition, triangleNormal, triangleUV);
    }
    {//Shadow Ray���
        if (TraceShadowRay(hitPosition, g_lightCB.position[0].xyz,payload.recursionDepth))
        {
            diffuseColor = diffuseColor * float3(0.1f, 0.1f, 0.1f);
            specullarColor = specullarColor * float3(0.1f, 0.1f, 0.1f);
        }
    }
    {//Reflection���(�߰� Radiance Ray�̿�)
        float3 nextRayDirection = reflect(WorldRayDirection(), triangleNormal);//�ݻ������ Trace�� ���� Ray�� Direction
        float4 reflectionColor = TraceRadianceRay(hitPosition, nextRayDirection, payload.recursionDepth);
        reflectedColor = mul(l_meshCB.reflectivity, reflectionColor.rgb);
        ambientColor = mul(1.f - l_meshCB.reflectivity, ambientColor);
        diffuseColor = mul(1.f - l_meshCB.reflectivity, diffuseColor);
        specullarColor = mul(1.f - l_meshCB.reflectivity, specullarColor);
    }
    float3 color = saturate(ambientColor + reflectedColor + specullarColor + diffuseColor);
    
    payload.color = float4(color, 1);
    
        
    
    

}