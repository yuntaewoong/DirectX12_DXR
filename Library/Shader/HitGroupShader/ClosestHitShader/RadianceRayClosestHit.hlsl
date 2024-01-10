#define HLSL
#include "../../Include/HLSLCommon.hlsli"
#include "../../Include/ClosestHitShaderCommon.hlsli"
#include "../../Include/RadianceRayTrace.hlsli"
#include "../../Include/ShadowRayTrace.hlsli"
#include "../../Include/RTAORayTrace.hlsli"
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
    
[shader("closesthit")]
void RadianceRayClosestHitShader(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
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
    
        
    float3 ambientColor = float3(0.1f, 0.1f, 0.1f);
    {//RTAO(Ray Tracing Ambient Occlusion 계산)
        ambientColor *= TraceRTAORay(hitPosition, triangleNormal, payload.recursionDepth);
    }
    
    float3 diffuseColor =l_meshCB.albedo.rgb;
    
    float3 specularColor = float3(1.f, 1.f, 1.f);
    if (l_meshCB.hasAlbedoTexture)
    { // 디퓨즈맵이 존재한다면 계산
        diffuseColor = l_albedoTexture.SampleLevel(l_sampler, triangleUV, 0).rgb;
    }
    
    if (l_meshCB.hasSpecularTexture)
    { //스페큘러 맵이 존재한다면 계산
        specularColor = l_specularTexture.SampleLevel(l_sampler, triangleUV, 0).xyz;
    }
    float3 pointToCamera = normalize(g_cameraCB.cameraPosition.xyz - hitPosition);
    float roughness = l_meshCB.roughness;
    if(l_meshCB.hasRoughnessTexture)
    {
        roughness = l_roughnessTexture.SampleLevel(l_sampler, triangleUV, 0).x;
    }
    float metallic = l_meshCB.metallic;
    if (l_meshCB.hasMetallicTexture)
    {
        metallic = l_metallicTexture.SampleLevel(l_sampler, triangleUV, 0).x;
    }
    float3 color = float3(0.f, 0.f, 0.f);
    [unroll(NUM_LIGHT)]
    for (uint i = 0; i < NUM_LIGHT; i++)
    {//직접광에 의한 Lighting
        float3 pointToLight = normalize(g_lightCB.position[i].xyz - hitPosition);
        float3 lightColor = float3(1.f, 1.f, 1.f);
        float lightIntensity = g_lightCB.lumen[i].r / (4 * PI * PI); 
        float lightDistance = sqrt(dot(g_lightCB.position[i].xyz - hitPosition, g_lightCB.position[i].xyz - hitPosition));
        float lightAttenuation = 1.0f / (1.0f + 0.09f * lightDistance + 0.032f * (lightDistance * lightDistance));
        float3 halfVector = normalize(pointToLight + pointToCamera);
        float3 diffuse = BxDF::BRDF::Diffuse::CalculateLambertianBRDF(diffuseColor);
        float3 F0 = 0.04f; //일반적인 프레넬 상수수치를 0.04로 정의
        F0 = lerp(F0, diffuseColor, metallic);
        float3 F = BxDF::BRDF::Specular::fresnelSchlick(max(dot(halfVector, pointToCamera), 0.0), F0); //반사정도 정의
        float3 kS = F; //Specular상수
        float3 kD = 1 - kS; //Diffuse 상수
        kD = kD * (1-metallic);//Diffuse에 metallic반영
        float NDF = BxDF::BRDF::Specular::DistributionGGX(triangleNormal, halfVector, roughness); //미세면 분포도 NDF계산
        float G = BxDF::BRDF::Specular::GeometrySmith(triangleNormal, pointToCamera, pointToLight, roughness); //미세면 그림자 계산
        float3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(triangleNormal, pointToCamera), 0.0) * max(dot(triangleNormal, pointToLight), 0.0) + 0.0001f;
        float3 specular = numerator / denominator;
        float shadow = TraceShadowRay(hitPosition, g_lightCB.position[i], payload.recursionDepth);
        float NdotL = max(dot(triangleNormal, pointToLight), 0.0);
        color += (1.f-shadow) * (kD * diffuse + specular) * lightColor *lightIntensity* lightAttenuation * NdotL;
    }
    {//간접광에 의한 Lighting(Roughness,Metallic기반으로 weight정하기)
    
        
        float3 nextRayDirection = reflect(WorldRayDirection(), triangleNormal); //반사용으로 Trace할 다음 Ray의 Direction
        float3 reflectedColor = TraceRadianceRay(hitPosition, nextRayDirection, payload.recursionDepth).rgb;
        float reflectedWeight = metallic * (1-roughness);//반사광 표현비율(metallic에 비례하고 roughness에 반비례하도록)
        float directWeight = 1 - reflectedWeight;
        
        color = color * directWeight + reflectedColor * reflectedWeight; //반사광(real time rendering을 위해 야매로 계산(정반사만 고려))
        color += ambientColor;//주변광
    }
    
    
    
    payload.color = float4(color, 1);
    
    
    
}