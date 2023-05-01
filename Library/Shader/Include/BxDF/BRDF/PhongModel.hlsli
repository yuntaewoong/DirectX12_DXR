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
        in float shadowAmount
    )
    {
        float3 ambient = float3(0.f, 0.f, 0.f);
        float3 diffuse = float3(0.f, 0.f, 0.f);
        float3 specular = float3(0.f, 0.f, 0.f);
        [unroll(NUM_LIGHT)]
        for (uint i = 0; i < NUM_LIGHT; i++)
        {
            ambient += ambientMap;
            float3 diffuseColor = BxDF::BRDF::Diffuse::CalculatePhongDiffuse(albedoMap, normal, pointToLights[i]);
            float3 specularColor = BxDF::BRDF::Specular::CalculateSpecular(specularMap, normal, pointToLights[i], pointToCamera);
            if (shadowAmount > 0.1f)
            {
                diffuseColor = diffuseColor * (1.f - shadowAmount);
                specularColor = specularColor * (1.f - shadowAmount);
            }
            diffuse += diffuseColor;
            specular += specularColor;
        }
        
        return saturate((ambient + diffuse + specular) * lightColor * lightAttenuation);
    }
}

#endif // PHONGMODEL