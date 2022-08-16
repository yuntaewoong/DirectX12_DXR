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
        hr = pDevice->CreateStateObject(m_stateObjectDesc, IID_PPV_ARGS(&m_stateObject));//����Ʈ���̽� Ŀ���� ���������� ����
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
            hitGroup->SetClosestHitShaderImport(CLOSEST_HIT_SHADER_NAMES[i]);//��Ʈ�׷�� ����� ���̴�������
            hitGroup->SetHitGroupExport(HIT_GROUP_NAMES[i]);                 //��Ʈ �׷� ����
            hitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);       //�� ��Ʈ�׷��� �ﰢ��
        }
    }
    void RaytracingPipelineStateObject::createShaderConfigSubobject()
    {
        CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT* shaderConfig = m_stateObjectDesc.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
        UINT payloadSize = max(sizeof(RayPayload), sizeof(ShadowRayPayload));   //  ���� ū���� �Ҵ�
        UINT attributeSize = sizeof(XMFLOAT2);                                  //  barycentrics
        shaderConfig->Config(payloadSize, attributeSize);                       //  payload, attribute������ ����(���̴����� ���ڷ� ����)
    }
    void RaytracingPipelineStateObject::createLocalRootSignatureSubobject(_In_ const ComPtr<ID3D12RootSignature>& pLocalRootSignature)
    {
        CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT* localRootSignature = m_stateObjectDesc.CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
        localRootSignature->SetRootSignature(pLocalRootSignature.Get());
        CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT* rootSignatureAssociation = m_stateObjectDesc.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
        rootSignatureAssociation->SetSubobjectToAssociate(*localRootSignature);
        rootSignatureAssociation->AddExport(HIT_GROUP_NAMES[RayType::Radiance]);//Radiance Hit Group���� ����ϰڴ�
    }
    void RaytracingPipelineStateObject::createGlobalRootSignatureSubobject(_In_ const ComPtr<ID3D12RootSignature>& pGlobalRootSignature)
    {
        CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT* globalRootSignature = m_stateObjectDesc.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();// �۷ι� ��Ʈ�ñ״�ó ���������Ʈ����
        globalRootSignature->SetRootSignature(pGlobalRootSignature.Get());
    }
    void RaytracingPipelineStateObject::createPipelineConfigSubobject()
    {
        CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT* pipelineConfig = m_stateObjectDesc.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();// ���������� config ���������Ʈ ���� 
        UINT maxRecursionDepth = MAX_RECURSION_DEPTH;//2���� recursion �ϰڴ�(1: �⺻ shading��, 2:shadow ray��)
        pipelineConfig->Config(maxRecursionDepth);//����
    }
}