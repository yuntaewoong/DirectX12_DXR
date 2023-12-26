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
            {//램버시안 표면의 난반사 BRDF계산
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
            {//프레넬 식(표면마다 다른 프레넬 상수(F0),Normal벡터와의 각도 cosTheta가 주어졌을때 반사되는 '비율'
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
                float NDF = DistributionGGX(normal, halfVector, roughness); //미세면 분포도 NDF계산
                float G = GeometrySmith(normal, pointToCamera, pointToLight, roughness); //미세면 그림자 계산
                
                float3 numerator = NDF * G * F;
                float denominator = 4.0 * max(dot(normal, pointToCamera), 0.0) * max(dot(normal, pointToLight), 0.0) + 0.0001f;
                float3 specular = numerator / denominator;
                
                return specular;
            }
        }
    }
}

#endif // PBRModel