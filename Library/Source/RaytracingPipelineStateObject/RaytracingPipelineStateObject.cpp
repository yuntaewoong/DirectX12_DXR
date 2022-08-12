#include "RaytracingPipelineStateObject\RaytracingPipelineStateObject.h"
#include "CompiledShaders\BasicRayTracing.hlsl.h"//�����ϵ� Shader

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
    void RaytracingPipelineStateObject::createDXILSubobject()
    {
        CD3DX12_DXIL_LIBRARY_SUBOBJECT* lib = m_stateObjectDesc.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();//���̴��� wrapping�ϴ� DXIL���̺귯�� ���������Ʈ ����
        D3D12_SHADER_BYTECODE libdxil = CD3DX12_SHADER_BYTECODE(static_cast<const void*>(g_pBasicRayTracing), ARRAYSIZE(g_pBasicRayTracing));//���� Ÿ�� ������ ���̴� ����Ʈ�ڵ� ��������
        lib->SetDXILLibrary(&libdxil);//DXIL-Raytacing Shader ����
        {
            lib->DefineExport(RAY_GEN_SHADER_NAME);//ray generation shader������ ����

            for (UINT i = 0; i < RayType::Count; i++)
            {
                lib->DefineExport(CLOSEST_HIT_SHADER_NAMES[i]); //closest hit shader�������� ����
                lib->DefineExport(MISS_SHADER_NAMES[i]);        //miss shader shader�������� ����
            }
        }
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