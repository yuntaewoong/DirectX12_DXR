#include"../../DataTypeSharedBothHLSLAndCPP.h"
#ifndef PBRMODEL
#define PBRMODEL


namespace BxDF
{
    namespace BRDF
    {
        namespace Diffuse
        {
            float3 CalculateLambertianBRDF(in float3 albedo)
            {//�����þ� ǥ���� ���ݻ� BRDF���
                return albedo / PI;
            }

        }
        namespace Specular
        {
            float DistributionGGX(in float3 normal,in float3 halfVector,in float roughness)
            {
                float a = roughness * roughness;
                float a2 = a * a;
                float NdotH = max(dot(normal, halfVector), 0.0);
                float NdotH2 = NdotH * NdotH;
	
                float num = a2;
                float denom = (NdotH2 * (a2 - 1.0) + 1.0);
                denom = PI * denom * denom;
	
                return num / denom;
            }

            float GeometrySchlickGGX(in float normalDotPointToCamera, in float roughness)
            {
                float r = (roughness + 1.0);
                float k = (r * r) / 8.0;

                float num = normalDotPointToCamera;
                float denom = normalDotPointToCamera * (1.0 - k) + k;
	
                return num / denom;
            }
            float GeometrySmith(in float3 normal,in float3 pointToCamera,in float3 pointToLight,in float roughness)
            {
                float normalDotPointToCamera = max(dot(normal, pointToCamera), 0.0);
                float normalDotPointToLight = max(dot(normal, pointToLight), 0.0);
                float ggx2 = GeometrySchlickGGX(normalDotPointToCamera, roughness);
                float ggx1 = GeometrySchlickGGX(normalDotPointToLight, roughness);
	
                return ggx1 * ggx2;
            }
            float3 fresnelSchlick(float cosTheta, float3 F0)
            {//������ ��(ǥ�鸶�� �ٸ� ������ ���(F0),Normal���Ϳ��� ���� cosTheta�� �־������� �ݻ�Ǵ� '����'
                return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
            }
            float3 CalculateCookTorranceBRDF(
                in float3 normal,
                in float3 pointToCamera,
                in float3 halfVector,
                in float3 pointToLight,
                in float roughness,
                in float3 F
            )
            {
                float NDF = DistributionGGX(normal, halfVector, roughness); //�̼��� ������ NDF���
                float G = GeometrySmith(normal, pointToCamera, pointToLight, roughness); //�̼��� �׸��� ���
                
                float3 numerator = NDF * G * F;
                float denominator = 4.0 * max(dot(normal, pointToCamera), 0.0) * max(dot(normal, pointToLight), 0.0) + 0.0001f;
                float3 specular = numerator / denominator;
                
                return specular;
            }
        }
    }
    float3 PBRShade(
        in float3 ambientMap,
        in float3 albedoMap,
        in float roughnessMap,
        in float metallicMap,
        in float3 normal,
        in float3 pointToLight,
        in float3 pointToCamera,
        in float3 lightColor,
        in float lightAttenuation,
        in bool isInShadow
    )
    {
        float3 halfVector = normalize(pointToLight+ pointToCamera);
        
        float3 ambient = ambientMap;
        float3 diffuse = BxDF::BRDF::Diffuse::CalculateLambertianBRDF(albedoMap);
        
        float3 F0 = float3(0.04f,0.04f,0.04f);//�Ϲ����� ������ �����ġ�� 0.04�� ����
        F0 = lerp(F0,albedoMap, metallicMap);
        float3 F = BxDF::BRDF::Specular::fresnelSchlick(max(dot(halfVector, pointToCamera), 0.0), F0); //�ݻ����� ����
        
        float3 kS = F;//Specular���
        float3 kD = float3(1.f, 1.f, 1.f) - kS;//Diffuse ���
        kD = kD * float3(1.f - metallicMap, 1.f - metallicMap, 1.f - metallicMap); //Diffuse�� metallic�ݿ�
        
        float3 specular = BxDF::BRDF::Specular::CalculateCookTorranceBRDF(normal, pointToCamera, halfVector, pointToLight, roughnessMap,F);
        if (isInShadow)
        {//�׸��� ȿ�� �ݿ�
            diffuse = diffuse * 0.1f;
            specular = float3(0.f, 0.f, 0.f);
        }
        float NdotL = max(dot(normal, pointToLight), 0.0);
        float3 color = ambient + (/*kD * */diffuse + specular) * lightColor * lightAttenuation* NdotL;
                
        //color = color / (color + float3(1.0f,1.0f,1.0f));
        //color = pow(color, float3(1.0f / 2.2f, 1.0f / 2.2f, 1.0f / 2.2f));
   
        return color;
        
    }
}

#endif // PBRModel