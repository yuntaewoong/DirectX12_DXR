#include "RaytracingPipelineStateObject\RaytracingPipelineStateObject.h"
#include "CompiledShaders\BasicRayTracing.hlsl.h"//컴파일된 Shader

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
        createDXILSubobject();
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
    void RaytracingPipelineStateObject::createDXILSubobject()
    {
        CD3DX12_DXIL_LIBRARY_SUBOBJECT* lib = m_stateObjectDesc.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();//셰이더를 wrapping하는 DXIL라이브러리 서브오브젝트 생성
        D3D12_SHADER_BYTECODE libdxil = CD3DX12_SHADER_BYTECODE(static_cast<const void*>(g_pBasicRayTracing), ARRAYSIZE(g_pBasicRayTracing));//빌드 타임 컴파일 셰이더 바이트코드 가져오기
        lib->SetDXILLibrary(&libdxil);//DXIL-Raytacing Shader 연결
        {
            lib->DefineExport(RAY_GEN_SHADER_NAME);//ray generation shader진입점 정의

            for (UINT i = 0; i < RayType::Count; i++)
            {
                lib->DefineExport(CLOSEST_HIT_SHADER_NAMES[i]); //closest hit shader진입점들 정의
                lib->DefineExport(MISS_SHADER_NAMES[i]);        //miss shader shader진입점들 정의
            }
        }
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