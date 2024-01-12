#define HLSL
#include "../../Include/HLSLCommon.hlsli"
#include "../../Include/ClosestHitShaderCommon.hlsli"
#include "../../Include/PathTracerRayTrace.hlsli"
#include "../../Include/BxDF/BRDF/PhongModel.hlsli"
#include "../../Include/BxDF/BRDF/PBRModel.hlsli"
    
/*================================================================================
    l_(variable_name) ===> local root signature�� ���ǵǴ� Resource(�� ������ Shader Table ���� �˸°� ����)
================================================================================*/
//SRV
StructuredBuffer<Vertex> l_vertices : register(t2, space0);
ByteAddressBuffer l_indices : register(t3, space0);
Texture2D l_albedoTexture : register(t4);
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

float3 RandomHemisphereVector(float random1, float random2)
{
    // random1�� random2�� [0, 1) ������ ����

    // theta�� phi ���� ���
    float theta = acos(random1); // [0, ��/2] ���� (�ݱ�)
    float phi = 2.0 * 3.14159265 * random2; // [0, 2��] ����

    // ���� ��ǥ�迡�� ī�׽þ� ��ǥ��� ��ȯ
    float x = sin(theta) * cos(phi);
    float y = sin(theta) * sin(phi);
    float z = cos(theta);

    return float3(x, y, z);
}
float3 RotateVectorToNormal(float3 inputVector, float3 normal)
{
    // ������ �������� ���� ��ǥ�� ����
    float3 nt;
    if (abs(normal.x) > abs(normal.z))
        nt = float3(-normal.y, normal.x, 0.0);
    else
        nt = float3(0.0, -normal.z, normal.y);

    nt = normalize(nt);
    float3 nb = normalize(cross(normal, nt));

    // ���͸� ������ �������� ȸ��
    return inputVector.x * nb + inputVector.y * nt + inputVector.z * normal;
}
float3 RandomVectorInHemisphere(float3 normal, float random1, float random2)
{
    float3 randomVector = RandomHemisphereVector(random1, random2);//�ݱ��������� Uniform���� ���� ����
    return RotateVectorToNormal(randomVector, normal);//��ǲ normal�������� ������ȯ
}


[shader("closesthit")]
void PathTracerRayClosestHitShader(inout PathTracerRayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    if(l_meshCB.emission > 0)
    {//Emissive�Ӽ��� material�� �������� ����ϱ⿡ �ٷ� ���� ����
        payload.color = l_meshCB.emission;
        return;
    }

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
    
    uint firstSeqLinearIndex = DispatchRaysIndex().x + 800 *  DispatchRaysIndex().y;
    uint seqLinearIndex0 = (firstSeqLinearIndex + payload.recursionDepth) % RANDOM_SEQUENCE_LENGTH;
	uint seqLinearIndex1 = (firstSeqLinearIndex + payload.recursionDepth) % RANDOM_SEQUENCE_LENGTH;
	float rand0 =  g_randomCB.randFloats0[seqLinearIndex0 / 4][seqLinearIndex0 % 4];
	float rand1 = g_randomCB.randFloats1[seqLinearIndex1 / 4][seqLinearIndex1 % 4];
    float3 randomVectorInHemisphere = RandomVectorInHemisphere(triangleNormal,rand0,rand1);
    
    
    
    float3 diffuse = l_meshCB.albedo.rgb;
    float roughness = l_meshCB.roughness;
    float metallic = l_meshCB.metallic;
    if (l_meshCB.hasAlbedoTexture)
    { 
        diffuse = l_albedoTexture.SampleLevel(l_sampler, triangleUV, 0).rgb;
    }
    if(l_meshCB.hasRoughnessTexture)
    {
        roughness = l_roughnessTexture.SampleLevel(l_sampler, triangleUV, 0).x;
    }
    if (l_meshCB.hasMetallicTexture)
    {
        metallic = l_metallicTexture.SampleLevel(l_sampler, triangleUV, 0).x;
    }
    float3 color = float3(0.f, 0.f, 0.f);
    float4 incomingLight = TracePathTracerRay(hitPosition,randomVectorInHemisphere,payload.recursionDepth);
    float3 pointToCamera = normalize(payload.camera - hitPosition);
        
    float3 halfVector = normalize(randomVectorInHemisphere + pointToCamera);
    diffuse = BxDF::BRDF::Diffuse::CalculateLambertianBRDF(diffuse);
    float3 F0 = float3(0.04f, 0.04f, 0.04f); //�Ϲ����� ������ �����ġ�� 0.04�� ����
    F0 = lerp(F0, diffuse, metallic);
    float3 F = BxDF::BRDF::Specular::fresnelSchlick(max(dot(halfVector, pointToCamera), 0.0), F0); //�ݻ����� ����
    float3 kS = F; //Specular���
    float3 kD = float3(1.f, 1.f, 1.f) - kS; //Diffuse ���
    kD = kD * (1-metallic);//Diffuse�� metallic�ݿ�
   
    float NDF = BxDF::BRDF::Specular::DistributionGGX(triangleNormal, halfVector, roughness); //�̼��� ������ NDF���
    float G = BxDF::BRDF::Specular::GeometrySmith(triangleNormal, pointToCamera, incomingLight.rgb, roughness); //�̼��� �׸��� ���
     
    float3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(triangleNormal, pointToCamera), 0.0) * max(dot(triangleNormal, incomingLight.rgb), 0.0) + 0.0001f;
    float3 specular = numerator / denominator;
   
   
    float NdotL = max(dot(triangleNormal, randomVectorInHemisphere), 0.0);
    color += (kD *diffuse + specular) * incomingLight.xyz /** lightAttenuation*/ * NdotL;
    
    payload.color = float4(color, 1);
    
}