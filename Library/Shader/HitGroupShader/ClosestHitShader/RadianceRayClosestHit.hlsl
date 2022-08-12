#define HLSL
#include "../../Include/HLSLCommon.hlsli"

/*================================================================================
    l_(variable_name) ===> local root signature로 정의되는 Resource(매 프레임 Shader Table 각각 알맞게 세팅)
================================================================================*/
ConstantBuffer<CubeConstantBuffer> l_cubeCB : register(b1);



//byteaddress buffer에서 3개의 index를 가져오는 함수
uint3 Load3x16BitIndices(uint offsetBytes)
{
    uint3 indices;
    
    //byte address buffer는 32비트 단위로 메모리를 load할수 있음
    //하지만 index데이터는 2바이트(16비트) * 3 = 6바이트(48비트)임
    //인덱스 데이터 3개를 얻기위해 64비트를 load한 후에 48비트를 취하는 방식을 사용
    const uint dwordAlignedOffset = offsetBytes & ~3;
    const uint2 four16BitIndices = g_indices.Load2(dwordAlignedOffset);
 
    
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

//보간할 vertex값, hit지점의 barycentric값을 넣어주면 보간 결과를 리턴
float3 HitAttribute(float3 vertexAttribute[3], BuiltInTriangleIntersectionAttributes attr)
{
    return vertexAttribute[0] +
        attr.barycentrics.x * (vertexAttribute[1] - vertexAttribute[0]) +
        attr.barycentrics.y * (vertexAttribute[2] - vertexAttribute[0]);
}

// Diffuse계산
float4 CalculateDiffuseLighting(float3 hitPosition, float3 normal)
{
    float3 pixelToLight = normalize(g_lightCB.position[0].xyz - hitPosition);
    float nDotL = max(0.0f, dot(pixelToLight, normal));
    return l_cubeCB.albedo * nDotL;
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
        TraceRayParameters::HitGroupOffset[RayType::Shadow], //hit group index(instance개수에 따라 달라짐)
        1, //shader table에서 geometry들 간의 간격(현재 각 instnance당 geometry는 1개라 딱히 의미없음)
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
    
    if (IsInShadow(hitPosition, g_lightCB.position[0].xyz))//shadow ray를 이용한 그림자 검사
    {
        payload.color = float4(0.1f, 0.1f, 0.1f, 1.f); //어두운 그림자
        return;
    }
    
    uint indexSizeInBytes = 2; //index는 16비트
    uint indicesPerTriangle = 3; //삼각형은 Vertex가 3개
    uint triangleIndexStride = indicesPerTriangle * indexSizeInBytes; //primitive별로 끊어 읽어야하는 Index단위
    uint baseIndex = PrimitiveIndex() * triangleIndexStride; //충돌한 Primitive에 해당하는 index값중 첫번째값
    
    const uint3 indices = Load3x16BitIndices(baseIndex); //base부터 3개의 index값 로드6

    //index값으로 vertex normal값 가져오기
    float3 vertexNormals[3] =
    {
        g_vertices[indices[0]].normal,
        g_vertices[indices[1]].normal,
        g_vertices[indices[2]].normal 
    };
 
    float3 triangleNormal = HitAttribute(vertexNormals, attr); //무게중심 좌표계로 normal값 보간하기
    float4 diffuseColor = CalculateDiffuseLighting(hitPosition, triangleNormal);
    float4 color = float4(0.2f, 0.2f, 0.2f, 1.f) * l_cubeCB.albedo + diffuseColor;
    payload.color = color;

}