#define HLSL
#include "../../Include/HLSLCommon.hlsli"



/*================================================================================
    l_(variable_name) ===> local root signature로 정의되는 Resource(매 프레임 Shader Table 각각 알맞게 세팅)
================================================================================*/
//SRV
StructuredBuffer<Vertex> l_vertices : register(t2, space0);
ByteAddressBuffer l_indices : register(t3, space0);
Texture2D l_diffuseTexture : register(t4);
//CBV
ConstantBuffer<RenderableConstantBuffer> l_renderableCB : register(b1);
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
// hit지점의 world 좌표 리턴
float3 HitWorldPosition()
{
    return WorldRayOrigin() + RayTCurrent() * WorldRayDirection();
    //WorldRayOrigin() => hit한 Ray의 origin좌표
    //RayTCurrent() => hit한 Ray의 t값(tx+b)
    //WorldRayDirection() => hit한 Ray의 direction
}

//보간할 vertex값, hit지점의 barycentric값을 넣어주면 보간 결과를 리턴(Float3버전)
float3 HitAttributeFloat3(float3 vertexAttribute[3], BuiltInTriangleIntersectionAttributes attr)
{
    return vertexAttribute[0] +
        attr.barycentrics.x * (vertexAttribute[1] - vertexAttribute[0]) +
        attr.barycentrics.y * (vertexAttribute[2] - vertexAttribute[0]);
}
//보간할 vertex값, hit지점의 barycentric값을 넣어주면 보간 결과를 리턴(Float2버전)
float2 HitAttributeFloat2(float2 vertexAttribute[3], BuiltInTriangleIntersectionAttributes attr)
{
    return vertexAttribute[0] +
        attr.barycentrics.x * (vertexAttribute[1] - vertexAttribute[0]) +
        attr.barycentrics.y * (vertexAttribute[2] - vertexAttribute[0]);
}

// Diffuse계산
float3 CalculateDiffuseLighting(float3 hitPosition, float3 normal,float2 uv)
{
    float3 pixelToLight = normalize(g_lightCB.position[0].xyz - hitPosition);
    float3 nDotL = max(0.0f, dot(pixelToLight, normal));
    float3 diffuseTexelColor = float3(1.f, 1.f, 1.f);
    if(l_renderableCB.hasTexture == 1)
    {
        diffuseTexelColor = l_diffuseTexture.SampleLevel(l_sampler, uv, 0).xyz; //Shadel Model lib 6_3에서는 Sample함수 컴파일에러남       
    }
    return l_renderableCB.albedo * nDotL * diffuseTexelColor;
}

// Specullar계산
float3 CalculateSpecullarLighting(float3 hitPosition, float3 normal, float2 uv)
{
    float3 lightToHit = normalize(hitPosition - g_lightCB.position[0].xyz);
    float3 cameraToHit = normalize(hitPosition - g_cameraCB.cameraPosition);
    float3 reflectDirection = normalize(reflect(lightToHit, normal));
    
    return pow(max(dot(-cameraToHit, reflectDirection), 0.0f), 15.0f) * l_renderableCB.albedo;
}


// Shadow Ray를 이용해 그림자이면 true, 아니면 false리턴
bool IsInShadow(in float3 hitPosition, in float3 lightPosition)
{
    RayDesc rayDesc;
    rayDesc.Direction = normalize(lightPosition - hitPosition); //shadow ray 방향은 hit지점->light지점
    rayDesc.Origin = hitPosition;
    rayDesc.TMin = 0.001f;
    rayDesc.TMax = 10000.f;
    
    ShadowRayPayload shadowPayload;
    shadowPayload.hit = 1.f; //closest hit시 값으로 초기화
    
    TraceRay(
        g_scene, //acceleration structure
        RAY_FLAG_SKIP_CLOSEST_HIT_SHADER, //closest hit shader무시(속도 향상)(나중에 필요해질때는 무시 안하게될듯)
        TraceRayParameters::InstanceMask, //instance mask딱히 설정 x
        TraceRayParameters::HitGroupOffset[RayType::Shadow], //hit group index
        TraceRayParameters::GeometryStride, //shader table에서 geometry들 간의 간격
        TraceRayParameters::MissShaderOffset[RayType::Shadow], //miss shader table에서 사용할 miss shader의 index
        rayDesc, //ray 정보
        shadowPayload //payload
    );
    return shadowPayload.hit > 0.5f; //0.5f보다 크다는 의미는 miss shader가 호출되지 않아서 그림자 영역에 존재한다는 의미
}

[shader("closesthit")]
void MyClosestHitShader(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    float3 hitPosition = HitWorldPosition();
    bool isInShadow = false;
    
    if (IsInShadow(hitPosition, g_lightCB.position[0].xyz))//shadow ray를 이용한 그림자 검사
    {
        isInShadow = true;
    }
    
    uint indexSizeInBytes = 2; //index는 16비트
    uint indicesPerTriangle = 3; //삼각형은 Vertex가 3개
    uint triangleIndexStride = indicesPerTriangle * indexSizeInBytes; //primitive별로 끊어 읽어야하는 Index단위
    uint baseIndex = PrimitiveIndex() * triangleIndexStride; //충돌한 Primitive에 해당하는 index값중 첫번째값
    
    const uint3 indices = Load3x16BitIndices(baseIndex); //base부터 3개의 index값 로드6

    float3 vertexNormals[3] =
    { //index값으로 world space vertex normal값 가져오기
        normalize(mul(float4(l_vertices[indices[0]].normal, 0), l_renderableCB.world).xyz),
        normalize(mul(float4(l_vertices[indices[1]].normal, 0), l_renderableCB.world).xyz),
        normalize(mul(float4(l_vertices[indices[2]].normal, 0), l_renderableCB.world).xyz)
    };
    float2 vertexUV[3] =
    { //index값으로 vertex uv값 가져오기
        l_vertices[indices[0]].uv,
        l_vertices[indices[1]].uv,
        l_vertices[indices[2]].uv 
    };
    
    
    float3 triangleNormal = HitAttributeFloat3(vertexNormals, attr); //무게중심 좌표계로 normal값 보간하기
    float2 triangleUV = HitAttributeFloat2(vertexUV, attr);//무게중심 좌표계로 UV값 보간하기
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