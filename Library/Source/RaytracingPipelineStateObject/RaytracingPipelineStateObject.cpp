#include "Common\pch.h"
#include "RaytracingPipelineStateObject\RaytracingPipelineStateObject.h"

#include "CompiledShaders\RadianceRayGeneration.hlsl.h"
#include "CompiledShaders\RadianceRayClosestHit.hlsl.h"
#include "CompiledShaders\ShadowRayClosestHit.hlsl.h"
#include "CompiledShaders\RadianceRayMiss.hlsl.h"
#include "CompiledShaders\ShadowRayMiss.hlsl.h"


namespace library
{
    RaytracingPipelineStateObject::RaytracingPipelineStateObject() :
        m_stateObject(nullptr),
        m_stateObjectDesc(D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE)
    {}
    HRESULT RaytracingPipelineStateObject::Initialize(
        _In_ const ComPtr<ID3D12Device5>& pDevice,
        _In_ const ComPtr<ID3D12RootSignature>& pLocalRootSignature,
        _In_ const ComPtr<ID3D12RootSignature>& pGlobalRootSignature
    )
    {
        HRESULT hr = S_OK;
        createDXILSubobjects();
        createHitGroupSubobject();
        createShaderConfigSubobject();
        createLocalRootSignatureSubobject(pLocalRootSignature);
        createGlobalRootSignatureSubobject(pGlobalRootSignature);
        createPipelineConfigSubobject();
        hr = pDevice->CreateStateObject(m_stateObjectDesc, IID_PPV_ARGS(&m_stateObject));//레이트레이싱 커스텀 파이프라인 생성
        if (FAILED(hr))
        {
            return hr;
        }
        return hr;
    }
    ComPtr<ID3D12StateObject>& RaytracingPipelineStateObject::GetStateObject()
    {
        return m_stateObject;
    }
    void RaytracingPipelineStateObject::createDXILSubobjects()
    {
        createDXILSubobject(static_cast<const void*>(g_pRadianceRayGeneration), ARRAYSIZE(g_pRadianceRayGeneration), RAY_GEN_SHADER_NAME);
        createDXILSubobject(static_cast<const void*>(g_pRadianceRayClosestHit), ARRAYSIZE(g_pRadianceRayClosestHit), CLOSEST_HIT_SHADER_NAMES[RayType::Radiance]);
        createDXILSubobject(static_cast<const void*>(g_pShadowRayClosestHit), ARRAYSIZE(g_pShadowRayClosestHit), CLOSEST_HIT_SHADER_NAMES[RayType::Shadow]);
        createDXILSubobject(static_cast<const void*>(g_pRadianceRayMiss), ARRAYSIZE(g_pRadianceRayMiss), MISS_SHADER_NAMES[RayType::Radiance]);
        createDXILSubobject(static_cast<const void*>(g_pShadowRayMiss), ARRAYSIZE(g_pShadowRayMiss), MISS_SHADER_NAMES[RayType::Shadow]);
    }
    void RaytracingPipelineStateObject::createDXILSubobject(_In_ const void* pShaderByteCode, _In_ SIZE_T shaderByteCodeSize, _In_ LPCWSTR entryPointName)
    {
        CD3DX12_DXIL_LIBRARY_SUBOBJECT* lib = m_stateObjectDesc.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
        D3D12_SHADER_BYTECODE libdxil = CD3DX12_SHADER_BYTECODE(pShaderByteCode, shaderByteCodeSize);
        lib->SetDXILLibrary(&libdxil);
        lib->DefineExport(entryPointName);
    }
    void RaytracingPipelineStateObject::createHitGroupSubobject()
    {
        for (UINT i = 0; i < RayType::Count; i++)
        {
            CD3DX12_HIT_GROUP_SUBOBJECT* hitGroup = m_stateObjectDesc.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
            hitGroup->SetClosestHitShaderImport(CLOSEST_HIT_SHADER_NAMES[i]);//히트그룹과 연결될 셰이더진입점
            hitGroup->SetHitGroupExport(HIT_GROUP_NAMES[i]);                 //히트 그룹 수출
            hitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);       //이 히트그룹은 삼각형
        }
    }
    void RaytracingPipelineStateObject::createShaderConfigSubobject()
    {
        CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT* shaderConfig = m_stateObjectDesc.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
        UINT payloadSize = max(sizeof(RayPayload), sizeof(ShadowRayPayload));   //  둘중 큰값을 할당
        UINT attributeSize = sizeof(XMFLOAT2);                                  //  barycentrics
        shaderConfig->Config(payloadSize, attributeSize);                       //  payload, attribute사이즈 정의(셰이더에서 인자로 사용됨)
    }
    void RaytracingPipelineStateObject::createLocalRootSignatureSubobject(_In_ const ComPtr<ID3D12RootSignature>& pLocalRootSignature)
    {
        CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT* localRootSignature = m_stateObjectDesc.CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
        localRootSignature->SetRootSignature(pLocalRootSignature.Get());
        CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT* rootSignatureAssociation = m_stateObjectDesc.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
        rootSignatureAssociation->SetSubobjectToAssociate(*localRootSignature);
        rootSignatureAssociation->AddExport(HIT_GROUP_NAMES[RayType::Radiance]);//Radiance Hit Group에서 사용하겠다
    }
    void RaytracingPipelineStateObject::createGlobalRootSignatureSubobject(_In_ const ComPtr<ID3D12RootSignature>& pGlobalRootSignature)
    {
        CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT* globalRootSignature = m_stateObjectDesc.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();// 글로벌 루트시그니처 서브오브젝트생성
        globalRootSignature->SetRootSignature(pGlobalRootSignature.Get());
    }
    void RaytracingPipelineStateObject::createPipelineConfigSubobject()
    {
        CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT* pipelineConfig = m_stateObjectDesc.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();// 파이프라인 config 서브오브젝트 생성 
        UINT maxRecursionDepth = MAX_RECURSION_DEPTH;//2번만 recursion 하겠다(1: 기본 shading용, 2:shadow ray용)
        pipelineConfig->Config(maxRecursionDepth);//적용
    }
}