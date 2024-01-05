
#include "pch.h"
#include "Scene\Scene.h"
#include "pbrtParser/Scene.h"


namespace library
{

    Scene::Scene() 
        :
        m_filePath(),
        m_meshes(),
        m_lights(),
        m_materials(),
        m_pointLightsConstantBuffer(nullptr),
        m_pointLightMappedData(nullptr)
    {}

    Scene::Scene(const std::filesystem::path & filePath)
        :
        m_filePath(filePath),
        m_meshes(),
        m_lights(),
        m_materials(),
        m_pointLightsConstantBuffer(nullptr),
        m_pointLightMappedData(nullptr)
    {}

    HRESULT Scene::Initialize(
        _In_ const ComPtr<ID3D12Device>& pDevice,
        _In_ const ComPtr<ID3D12CommandQueue>& pCommandQueue,
        _In_ CBVSRVUAVDescriptorHeap& cbvSrvUavDescriptorHeap
    )
    {
        HRESULT hr = S_OK;
        if (!m_filePath.empty())
        {//파일경로가 주어졌다면, pbrt 씬 로딩
            std::shared_ptr<pbrt::Scene> pbrtScene = pbrt::importPBRT(m_filePath.string());
            traverse(pbrtScene->world);
            
        }


        for (UINT i = 0; i < m_meshes.size(); i++)
        {
            hr = m_meshes[i]->Initialize(pDevice);
            if (FAILED(hr))
            {
                return hr;
            }
        }
        for (UINT i = 0; i < m_models.size(); i++)
        {
            hr = m_models[i]->Initialize(pDevice, pCommandQueue,cbvSrvUavDescriptorHeap);
            if (FAILED(hr))
            {
                return hr;
            }
            const std::vector<std::shared_ptr<Mesh>>& meshes = m_models[i]->GetMeshes();
            for (UINT i = 0u; i < meshes.size(); i++)
            {//모델들을 구성하는 Mesh를 씬에 추가
                m_meshes.push_back(meshes[i]);
            }
        }
        for (UINT i = 0; i < m_lights.size(); i++)
        {
            hr = m_lights[i]->Initialize(pDevice);
            if (FAILED(hr))
            {
                return hr;
            }
        }
        for (UINT i = 0; i < m_materials.size(); i++)
        {
            hr = m_materials[i]->Initialize(pDevice, pCommandQueue, cbvSrvUavDescriptorHeap);
            if (FAILED(hr))
            {
                return hr;
            }
        }
        hr = createLightConstantBuffer(pDevice);
        if (FAILED(hr))
        {
            return hr;
        }
        return hr;
    }
    void Scene::AddMesh(_In_ const std::shared_ptr<Mesh>& pMesh)
    {
        m_meshes.push_back(pMesh);
    }
    void Scene::AddModel(const std::shared_ptr<Model>& pModel)
    {
        m_models.push_back(pModel);
    }
    void Scene::AddMaterial(_In_ const std::shared_ptr<Material>& pMaterial)
    {
        m_materials.push_back(pMaterial);
    }
    const std::vector<std::shared_ptr<Mesh>>& Scene::GetMeshes() const
    {
        return m_meshes;
    }
    void Scene::AddLight(_In_ const std::shared_ptr<PointLight>& pLight)
    {
        m_lights.push_back(pLight);
    }
    void Scene::Update(_In_ FLOAT deltaTime)
    {
        for (UINT i = 0; i < m_meshes.size(); i++)
        {
            m_meshes[i]->Update(deltaTime);
        }
        for (UINT i = 0; i < m_lights.size(); i++)
        {
            m_lights[i]->Update(deltaTime);
        }
        /*PointLightConstantBuffer cb = {};
        for (UINT i = 0; i < m_lights.size(); i++)
        {
            cb.position[i] = m_lights[i]->GetPosition();
        }
        memcpy(m_pointLightMappedData, &cb, sizeof(cb));*/
    }
    ComPtr<ID3D12Resource>& Scene::GetPointLightsConstantBuffer()
    {
        return m_pointLightsConstantBuffer;
    }
    void Scene::traverse(std::shared_ptr<pbrt::Object> object)
    {
        
        for (auto shape : object->shapes) 
        {//shape: pbrt씬의 primitive구조
            if (std::shared_ptr<pbrt::TriangleMesh> mesh = std::dynamic_pointer_cast<pbrt::TriangleMesh>(shape))
            {//TriangleMesh로 다운캐스팅
                m_meshes.push_back(
                    std::make_shared<Mesh>(XMVectorZero(),XMVectorZero(),XMVectorSet(1.f,1.f,1.f,1.f), XMFLOAT4(1.f,1.f,0.f,.1f))
                );
                {//디버그용(모든 primitive에 기본 머테리얼 대응)
                    m_materials.push_back(std::make_shared<Material>(MaterialType::PBR));
                    m_meshes[m_meshes.size() - 1]->SetMaterial(m_materials[m_materials.size() - 1]);
                }
                {//vertex로딩
                    for (UINT i = 0; i < static_cast<UINT>(mesh->vertex.size()); i++)
                    {
                        const pbrt::vec3f& pbrtPosition = mesh->vertex[i];
                        //const pbrt::vec2f& pbrtUV = mesh->texcoord[i];//.pbrt파일은 텍스처가 없는경우 uv값이 주어지지 않음
                        const pbrt::vec3f& pbrtNormal = mesh->normal[i];
                        Vertex vertex =
                        {
                            .position = XMFLOAT3(pbrtPosition.x, pbrtPosition.y, pbrtPosition.z),
                            .uv = XMFLOAT2(0.f, 0.f),
                            .normal = XMFLOAT3(pbrtNormal.x, pbrtNormal.y, pbrtNormal.z),
                            .tangent = XMFLOAT3(0.f,0.f,0.f),//
                            .biTangent = XMFLOAT3(0.f,0.f,0.f)
                        };//pbrt에는 노말맵계산을 위한 tangent,bitangent정보가 들어있지 않다
                        m_meshes[m_meshes.size() - 1]->AddVertex(vertex);
                    }
                }
                {//index로딩
                    for (UINT i = 0u; i < static_cast<UINT>(mesh->index.size()); i++)
                    {
                        const pbrt::vec3i& pbrtIndex = mesh->index[i];
                        Index aIndices[3] =
                        {
                            static_cast<Index>(pbrtIndex.x),
                            static_cast<Index>(pbrtIndex.y),
                            static_cast<Index>(pbrtIndex.z)
                        };
                        m_meshes[m_meshes.size() - 1]->AddIndex(aIndices[0]);
                        m_meshes[m_meshes.size() - 1]->AddIndex(aIndices[1]);
                        m_meshes[m_meshes.size() - 1]->AddIndex(aIndices[2]);
                    }
                }
            }
        }
        for (auto inst : object->instances) 
        {//Instance들에 대하여 재귀호출
            traverse(inst->object);
        }
    }
    HRESULT Scene::createLightConstantBuffer(_In_ const ComPtr<ID3D12Device>& pDevice)
    {//light들에 대한 constant buffer만들기
        HRESULT hr = S_OK;
        const D3D12_HEAP_PROPERTIES uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

        size_t cbSize = 256;//왜인지는 모르겠지만 Constant버퍼는 256의 배수여야함
        const D3D12_RESOURCE_DESC constantBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(cbSize);

        hr = pDevice->CreateCommittedResource(
            &uploadHeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &constantBufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_pointLightsConstantBuffer)
        );
        if (FAILED(hr))
        {
            return hr;
        }
        CD3DX12_RANGE readRange(0, 0);
        hr = m_pointLightsConstantBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_pointLightMappedData));
        if (FAILED(hr))
        {
            return hr;
        }
        updateLightConstantBuffer();
        return hr;
    }
    void Scene::updateLightConstantBuffer()
    {
        PointLightConstantBuffer cb = {};
        for (UINT i = 0; i < m_lights.size(); i++)
        {
            cb.position[i] = m_lights[i]->GetPosition();
        }
        memcpy(m_pointLightMappedData, &cb, sizeof(cb));
    }
}