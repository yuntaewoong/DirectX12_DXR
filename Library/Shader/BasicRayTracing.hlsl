/*================================================================
�������� Error List�鿡 �ߴ� Error���� Intellisense�� ��û�ؼ� ��� ����, �Ű澲�� ����

==================================================================*/


#ifndef RAYTRACING_HLSL
#define RAYTRACING_HLSL

#define HLSL
#include "Include/DataTypeSharedBothHLSLAndCPP.h"

#define NUM_LIGHTS 2

/*=================================================================
    g_(variable_name) ===> global root signature�� ���ǵǴ� Resource(�� ������ 1���� ����)
==================================================================*/
RaytracingAccelerationStructure g_scene : register(t0, space0);
RWTexture2D<float4> g_renderTarget : register(u0);
ByteAddressBuffer g_indices : register(t1, space0);
StructuredBuffer<Vertex> g_vertices : register(t2, space0);
ConstantBuffer<CameraConstantBuffer> g_cameraCB : register(b0);
ConstantBuffer<PointLightConstantBuffer> g_lightCB : register(b2);

/*================================================================================
    l_(variable_name) ===> local root signature�� ���ǵǴ� Resource(�� ������ Shader Table ���� �˸°� ����)
================================================================================*/
ConstantBuffer<CubeConstantBuffer> l_cubeCB : register(b1);

//byteaddress buffer���� 3���� index�� �������� �Լ�
uint3 Load3x16BitIndices(uint offsetBytes)
{
    uint3 indices;
    
    //byte address buffer�� 32��Ʈ ������ �޸𸮸� load�Ҽ� ����
    //������ index�����ʹ� 2����Ʈ(16��Ʈ) * 3 = 6����Ʈ(48��Ʈ)��
    //�ε��� ������ 3���� ������� 64��Ʈ�� load�� �Ŀ� 48��Ʈ�� ���ϴ� ����� ���
    const uint dwordAlignedOffset = offsetBytes & ~3;
    const uint2 four16BitIndices = g_indices.Load2(dwordAlignedOffset);
 
    
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

// Screen Space Ray�� World Space Ray�� ��ȯ
// index: screen ��ǥ(�ػ� ���� ����), origin: world��ǥ,  direction: world����
inline void GenerateCameraRay(uint2 index, out float3 origin, out float3 direction)
{
    float2 xy = index + 0.5f; // �ȼ��� ���� ä��
    float2 screenPos = xy / DispatchRaysDimensions().xy * 2.0 - 1.0;//NDC��ǥ��� ��ȯ(�� ����: -1 ~ 1)

    screenPos.y = -screenPos.y;//directX �� LHS��ǥ��� �ȼ� index���� y���� ������ �ٸ��� ������ ����
    
    float4 world = mul(float4(screenPos, 0, 1), g_cameraCB.projectionToWorld);//NDC��ǥ�� Ray�� �������� World Space�� ��ȯ

    world.xyz /= world.w;
    origin = g_cameraCB.cameraPosition.xyz;//origin�� ī�޶� ��ǥ
    direction = normalize(world.xyz - origin);//direction�� world-camera
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
    rayDesc.Direction = normalize(lightPosition - hitPosition);//shadow ray ������ hit����->light����
    rayDesc.Origin = hitPosition;
    rayDesc.TMin = 0.001f;
    rayDesc.TMax = 10000.f;
    
    ShadowRayPayload shadowPayload;
    shadowPayload.hit = 1.f;//closest hit�� ������ �ʱ�ȭ
    
    TraceRay(
        g_scene,                                               //acceleration structure
        RAY_FLAG_SKIP_CLOSEST_HIT_SHADER,                      //closest hit shader����(�ӵ� ���)(���߿� �ʿ��������� ���� ���ϰԵɵ�)
        TraceRayParameters::InstanceMask,                      //instance mask���� ���� x
        TraceRayParameters::HitGroupOffset[RayType::Shadow],   //hit group index(instance������ ���� �޶���)
        1,                                                     //shader table���� geometry�� ���� ����(���� �� instnance�� geometry�� 1���� ���� �ǹ̾���)
        TraceRayParameters::MissShaderOffset[RayType::Shadow], //miss shader table���� ����� miss shader�� index
        rayDesc,                                               //ray ����
        shadowPayload                                          //payload
    );
    return shadowPayload.hit > 0.5f;//0.5f���� ũ�ٴ� �ǹ̴� miss shader�� ȣ����� �ʾƼ� �׸��� ������ �����Ѵٴ� �ǹ�
}

[shader("raygeneration")]
void MyRaygenShader()
{
    float3 rayDir;
    float3 origin;
    GenerateCameraRay(DispatchRaysIndex().xy, origin, rayDir);//World Space Ray����
    
    
    RayDesc ray;
    ray.Origin = origin;
    ray.Direction = rayDir;
    ray.TMin = 0.001f;
    ray.TMax = 10000.0;
    RayPayload payload = { float4(0, 0, 0, 0) };
    TraceRay(
        g_scene,                                                    //acceleration structure
        RAY_FLAG_NONE,                                              //���� �÷��׸� ���� ����
        TraceRayParameters::InstanceMask,                           //instance mask
        TraceRayParameters::HitGroupOffset[RayType::Radiance],       //hit group base index����(����:trace ray���� index+ instance index + (geometry index * geometry stride)
        1,                                                          //geometry stride, �� ���������� ���� �Ű�Ƚᵵ��
        TraceRayParameters::MissShaderOffset[RayType::Radiance],     //miss shader index, 0���� �⺻ miss shader
        ray,                                                        //ray ����
        payload                                                     //payload
    );
 
    g_renderTarget[DispatchRaysIndex().xy] = payload.color;

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
    
    uint indexSizeInBytes = 2;//index�� 16��Ʈ
    uint indicesPerTriangle = 3;//�ﰢ���� Vertex�� 3��
    uint triangleIndexStride = indicesPerTriangle * indexSizeInBytes;//primitive���� ���� �о���ϴ� Index����
    uint baseIndex = PrimitiveIndex() * triangleIndexStride;//�浹�� Primitive�� �ش��ϴ� index���� ù��°��
    
    const uint3 indices = Load3x16BitIndices(baseIndex);//base���� 3���� index�� �ε�

    //index������ vertex normal�� ��������
    float3 vertexNormals[3] =
    {
        g_vertices[indices[0]].normal,
        g_vertices[indices[1]].normal,
        g_vertices[indices[2]].normal 
    };
 
    float3 triangleNormal = HitAttribute(vertexNormals, attr);//�����߽� ��ǥ��� normal�� �����ϱ�
    float4 diffuseColor = CalculateDiffuseLighting(hitPosition, triangleNormal);
    float4 color = float4(0.2f, 0.2f, 0.2f, 1.f) * l_cubeCB.albedo + diffuseColor;
    payload.color = color;

}
[shader("miss")]
void MyMissShader(inout RayPayload payload)
{
    payload.color = float4(0.0f, 0.2f, 0.4f, 1.0f);
}

[shader("closesthit")]
void MyShadowRayClosestHitShader(inout ShadowRayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    payload.hit = 1.f; //�¾Ҵ�!(�׸���x)
}
[shader("miss")]
void MyShadowRayMissShader(inout ShadowRayPayload payload)
{
    payload.hit = 0.f; //�ȸ¾Ҵ�!(�׸���o)
}

#endif // RAYTRACING_HLSL