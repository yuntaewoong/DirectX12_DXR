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
            float DistributionGGX(float3 N, float3 H, float roughness)
            {
                float a = roughness * roughness;
                float a2 = a * a;
                float NdotH = max(dot(N, H), 0.0);
                float NdotH2 = NdotH * NdotH;
	
                float num = a2;
                float denom = (NdotH2 * (a2 - 1.0) + 1.0);
                denom = PI * denom * denom;
	
                return num / denom;
            }

            float GeometrySchlickGGX(float NdotV, float roughness)
            {
                float r = (roughness + 1.0);
                float k = (r * r) / 8.0;

                float num = NdotV;
                float denom = NdotV * (1.0 - k) + k;
	
                return num / denom;
            }
            float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
            {
                float NdotV = max(dot(N, V), 0.0);
                float NdotL = max(dot(N, L), 0.0);
                float ggx2 = GeometrySchlickGGX(NdotV, roughness);
                float ggx1 = GeometrySchlickGGX(NdotL, roughness);
	
                return ggx1 * ggx2;
            }
            float3 fresnelSchlick(float cosTheta, float3 F0)
            {//������ ��(ǥ�鸶�� �ٸ� ������ ���(F0),Normal���Ϳ��� ���� cosTheta�� �־������� �ݻ�Ǵ� '����'
                return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
            }
            float3 CalculateCookTorranceBRDF(in float3 normal,in float3 pointToCamera,in float3 halfVector,in float3 pointToLight,in float roughness,in float3 F)
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
        in float3 roughnessMap,
        in float3 metallicMap,
        in float3 normal,
        in float3 pointToLight,
        in float3 pointToCamera,
        in float3 lightColor,
        in float lightAttenuation,
        in bool isInShadow
    )
    {
        float3 halfVector = normalize(pointToCamera + pointToCamera);
        
        float3 ambient = ambientMap;
        float3 diffuse = BxDF::BRDF::Diffuse::CalculateLambertianBRDF(albedoMap);
        
        float3 F0 = float3(0.04f,0.04f,0.04f);//�Ϲ����� ������ �����ġ�� 0.04�� ����
        F0 = lerp(F0, albedoMap, metallicMap);
        float3 F = BxDF::BRDF::Specular::fresnelSchlick(max(dot(halfVector, pointToCamera), 0.0), F0); //�ݻ����� ����
        
        float3 kS = F;//Specular���
        float3 kD = float3(1.f, 1.f, 1.f) - kS;//Diffuse ���
        kD *= 1.f - metallicMap;//Diffuse�� metallic�ݿ�
        
        float3 specular = BxDF::BRDF::Specular::CalculateCookTorranceBRDF(normal, pointToCamera, halfVector, pointToLight, roughnessMap,F);
        if (isInShadow)
        {
            diffuse = diffuse * 0.1f;
            specular = float3(0.f, 0.f, 0.f);
        }
        float NdotL = max(dot(normal, pointToLight), 0.0);
        return ambient + (kD * diffuse + specular) * lightColor * lightAttenuation * NdotL;
    }
}

#endif // PBRModel