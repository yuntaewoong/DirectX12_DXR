#pragma once
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