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
            float3 CalculateSpecular(in float3 specularMap, in float3 normal, in float3 pointToLight, in float3 pointToCamera)
            {//Specular BRDF계산
                float3 reflectDirection = normalize(reflect(-pointToLight, normal));
                float cosFactor = 15.0f;
                return specularMap * pow(max(dot(pointToCamera, reflectDirection), 0.0f), cosFactor);
            }
        }
    }
    float3 PhongShade(
        in float3 ambientMap,
        in float3 albedoMap,
        in float3 specularMap,
        in float3 normal, 
        in float3 pointToLights[NUM_LIGHT],
        in float3 pointToCamera,
        in float3 lightColor,
        in float  lightAttenuation,
        in bool isInShadow
    )
    {
        float3 color = float3(0.f, 0.f, 0.f);
        [unroll(NUM_LIGHT)]
        for (uint i = 0; i < NUM_LIGHT; i++)
        {
            float3 ambient = ambientMap;
            float3 diffuse = BxDF::BRDF::Diffuse::CalculatePhongDiffuse(albedoMap, normal, pointToLights[i]);
            float3 specular = BxDF::BRDF::Specular::CalculateSpecular(specularMap, normal, pointToLights[i], pointToCamera);
            if (isInShadow)
            {
                diffuse = diffuse * 0.1f;
                specular = float3(0.f, 0.f, 0.f);
            }
            color += ambient + diffuse + specular;
        }
        
        return saturate(color * lightColor * lightAttenuation);
    }
}

#endif // PHONGMODEL