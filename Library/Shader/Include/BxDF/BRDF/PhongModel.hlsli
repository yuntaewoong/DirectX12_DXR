#include"../../DataTypeSharedBothHLSLAndCPP.h"
#ifndef PHONGMODEL
#define PHONGMODEL
namespace BxDF
{
    namespace BRDF
    {
        namespace Diffuse
        {
            float3 CalculatePhongDiffuse(in float3 albedo, in float3 normal, in float3 pointToLight)
            { //Diffuse BRDF계산
                float3 nDotL = max(0.0f, dot(pointToLight, normal));
                return albedo * nDotL;
            }
        }
        namespace Specular
        {
            float3 CalculatePhongSpecular(in float3 specularMap, in float3 normal, in float3 pointToLight, in float3 pointToCamera)
            {//Specular BRDF계산
                float3 reflectDirection = normalize(reflect(-pointToLight, normal));
                float cosFactor = 15.0f;
                return specularMap * pow(max(dot(pointToCamera, reflectDirection), 0.0f), cosFactor);
            }
        }
    }
}

#endif // PHONGMODEL