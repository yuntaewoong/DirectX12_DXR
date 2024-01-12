#define HLSL
#include "../../Include/HLSLCommon.hlsli"
#include "../../Include/ClosestHitShaderCommon.hlsli"
#include "../../Include/PathTracerRayTrace.hlsli"
#include "../../Include/BxDF/BRDF/PhongModel.hlsli"
#include "../../Include/BxDF/BRDF/PBRModel.hlsli"
    
/*================================================================================
    l_(variable_name) ===> local root signature로 정의되는 Resource(매 프레임 Shader Table 각각 알맞게 세팅)
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


//byteaddress buffer에서 3개의 index를 가져오는 함수
uint3 Load3x16BitIndices(uint offsetBytes)
{
    uint3 indices;
    
    //byte address buffer는 32비트 단위로 메모리를 load할수 있음
    //하지만 index데이터는 2바이트(16비트) * 3 = 6바이트(48비트)임
    //인덱스 데이터 3개를 얻기위해 64비트를 load한 후에 48비트를 취하는 방식을 사용
    const uint dwordAlignedOffset = offsetBytes & ~3;
    const uint2 four16BitIndices = l_indices.Load2(dwordAlignedOffset);
 
    
    if (dwordAlignedOffset == offsetBytes)
    { //{ 0 1 | 2 - } 꼴
        indices.x = four16BitIndices.x & 0xffff;
        indices.y = (four16BitIndices.x >> 16) & 0xffff;
        indices.z = four16BitIndices.y & 0xffff;
    }
    else
    { //{ - 0 | 1 2 } 꼴
        indices.x = (four16BitIndices.x >> 16) & 0xffff;
        indices.y = four16BitIndices.y & 0xffff;
        indices.z = (four16BitIndices.y >> 16) & 0xffff;
    }

    return indices;
}


//노말맵이 적용된 새로운 노말값 리턴
float3 CalculateNormalmapNormal(float3 originNormal,float3 tangent,float3 biTangent,float2 uv)
{
    if(!l_meshCB.hasNormalTexture)
        return originNormal;
    float3 newNormal = l_normalTexture.SampleLevel(l_sampler, uv, 0).xyz;
    newNormal = (newNormal * 2.0f) - 1.0f;
    newNormal = (newNormal.x * tangent) + (newNormal.y * biTangent) + (newNormal.z * originNormal);//TBN변환
    return normalize(newNormal);
}

float3 RandomHemisphereVector(float random1, float random2)
{
    // random1과 random2는 [0, 1) 범위의 난수

    // theta와 phi 각도 계산
    float theta = acos(random1); // [0, π/2] 범위 (반구)
    float phi = 2.0 * 3.14159265 * random2; // [0, 2π] 범위

    // 구면 좌표계에서 카테시안 좌표계로 변환
    float x = sin(theta) * cos(phi);
    float y = sin(theta) * sin(phi);
    float z = cos(theta);

    return float3(x, y, z);
}
float3 RotateVectorToNormal(float3 inputVector, float3 normal)
{
    // 법선을 기준으로 직교 좌표계 생성
    float3 nt;
    if (abs(normal.x) > abs(normal.z))
        nt = float3(-normal.y, normal.x, 0.0);
    else
        nt = float3(0.0, -normal.z, normal.y);

    nt = normalize(nt);
    float3 nb = normalize(cross(normal, nt));

    // 벡터를 법선을 기준으로 회전
    return inputVector.x * nb + inputVector.y * nt + inputVector.z * normal;
}
float3 RandomVectorInHemisphere(float3 normal, float random1, float random2)
{
    float3 randomVector = RandomHemisphereVector(random1, random2);//반구공간상의 Uniform랜덤 벡터 생성
    return RotateVectorToNormal(randomVector, normal);//인풋 normal기준으로 공간변환
}


[shader("closesthit")]
void PathTracerRayClosestHitShader(inout PathTracerRayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    if(l_meshCB.emission > 0)
    {//Emissive속성의 material은 광원으로 취급하기에 바로 색상 리턴
        payload.color = l_meshCB.emission;
        return;
    }

    float3 hitPosition = HitWorldPosition();
    uint indexSizeInBytes = 2; //index는 16비트
    uint indicesPerTriangle = 3; //삼각형은 Vertex가 3개
    uint triangleIndexStride = indicesPerTriangle * indexSizeInBytes; //primitive별로 끊어 읽어야하는 Index단위
    uint baseIndex = PrimitiveIndex() * triangleIndexStride; //충돌한 Primitive에 해당하는 index값중 첫번째값
    
    const uint3 indices = Load3x16BitIndices(baseIndex); //base부터 3개의 index값 로드6

    float3 vertexNormals[3] =
    { //index값으로 world space vertex normal값 가져오기
        normalize(mul(float4(l_vertices[indices[0]].normal, 0), l_meshCB.world).xyz),
        normalize(mul(float4(l_vertices[indices[1]].normal, 0), l_meshCB.world).xyz),
        normalize(mul(float4(l_vertices[indices[2]].normal, 0), l_meshCB.world).xyz)
    };
    float3 vertexTangent[3] =
    { //index값으로 world space vertex tangent값 가져오기
        normalize(mul(float4(l_vertices[indices[0]].tangent, 0), l_meshCB.world).xyz),
        normalize(mul(float4(l_vertices[indices[1]].tangent, 0), l_meshCB.world).xyz),
        normalize(mul(float4(l_vertices[indices[2]].tangent, 0), l_meshCB.world).xyz)
    };
    float3 vertexBiTangent[3] =
    { //index값으로 world space vertex biTangent값 가져오기
        normalize(mul(float4(l_vertices[indices[0]].biTangent, 0), l_meshCB.world).xyz),
        normalize(mul(float4(l_vertices[indices[1]].biTangent, 0), l_meshCB.world).xyz),
        normalize(mul(float4(l_vertices[indices[2]].biTangent, 0), l_meshCB.world).xyz)
    };
    float2 vertexUV[3] =
    { //index값으로 vertex uv값 가져오기
        l_vertices[indices[0]].uv,
        l_vertices[indices[1]].uv,
        l_vertices[indices[2]].uv 
    };
    
    float3 triangleNormal = HitAttributeFloat3(vertexNormals, attr); //무게중심 좌표계로 normal값 보간하기
    float3 triangleTangent = HitAttributeFloat3(vertexTangent, attr); //무게중심 좌표계로 tangent값 보간하기
    float3 triangleBitangent = HitAttributeFloat3(vertexBiTangent, attr); //무게중심 좌표계로 biTangent값 보간하기
    float2 triangleUV = HitAttributeFloat2(vertexUV, attr); //무게중심 좌표계로 UV값 보간하기
    triangleNormal = CalculateNormalmapNormal(triangleNormal, triangleTangent, triangleBitangent, triangleUV); //노말맵이 있다면 노말맵적용
    
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
    float3 F0 = float3(0.04f, 0.04f, 0.04f); //일반적인 프레넬 상수수치를 0.04로 정의
    F0 = lerp(F0, diffuse, metallic);
    float3 F = BxDF::BRDF::Specular::fresnelSchlick(max(dot(halfVector, pointToCamera), 0.0), F0); //반사정도 정의
    float3 kS = F; //Specular상수
    float3 kD = float3(1.f, 1.f, 1.f) - kS; //Diffuse 상수
    kD = kD * (1-metallic);//Diffuse에 metallic반영
   
    float NDF = BxDF::BRDF::Specular::DistributionGGX(triangleNormal, halfVector, roughness); //미세면 분포도 NDF계산
    float G = BxDF::BRDF::Specular::GeometrySmith(triangleNormal, pointToCamera, incomingLight.rgb, roughness); //미세면 그림자 계산
     
    float3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(triangleNormal, pointToCamera), 0.0) * max(dot(triangleNormal, incomingLight.rgb), 0.0) + 0.0001f;
    float3 specular = numerator / denominator;
   
   
    float NdotL = max(dot(triangleNormal, randomVectorInHemisphere), 0.0);
    color += (kD *diffuse + specular) * incomingLight.xyz /** lightAttenuation*/ * NdotL;
    
    payload.color = float4(color, 1);
    
}