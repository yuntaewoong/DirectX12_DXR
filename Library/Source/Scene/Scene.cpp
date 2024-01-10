
#include "pch.h"
#include "Scene\Scene.h"



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
            loadPBRT(pbrtScene->world);
            
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
        updateLightConstantBuffer();
    }
    ComPtr<ID3D12Resource>& Scene::GetPointLightsConstantBuffer()
    {
        return m_pointLightsConstantBuffer;
    }
    void Scene::loadPBRT(_In_ const std::shared_ptr<const pbrt::Object> object)
    {
        
        for (auto shape : object->shapes) 
        {//shape: pbrt씬의 primitive구조
            if (std::shared_ptr<pbrt::AreaLight> areaLight = std::dynamic_pointer_cast<pbrt::AreaLight>(shape->areaLight))
            {//AreaLight로 다운캐스팅성공시
                int a = 1;
            }
            if (std::shared_ptr<pbrt::TriangleMesh> mesh = std::dynamic_pointer_cast<pbrt::TriangleMesh>(shape))
            {//TriangleMesh로 다운캐스팅 성공시 Mesh로딩
                loadPBRTTriangleMesh(mesh);
                loadPBRTMaterial(mesh->material);
                m_meshes[m_meshes.size() - 1]->SetMaterial(m_materials[m_materials.size() - 1]);//로딩된 mesh-material 대응
            }
        }
        for (auto inst : object->instances) 
        {//Instance들에 대하여 재귀호출
            loadPBRT(inst->object);
        }
    }
    void Scene::loadPBRTMaterial(_In_ const std::shared_ptr<const pbrt::Material> material)
    {
        m_materials.push_back(std::make_shared<Material>());
    }
    void Scene::loadPBRTTriangleMesh(_In_ const std::shared_ptr<const pbrt::TriangleMesh> mesh)
    {
        m_meshes.push_back(
            std::make_shared<Mesh>(
                XMVectorZero(),
                XMVectorZero(),
                XMVectorSet(1.f,1.f,1.f,1.f)
            )
        );//로딩전 빈 mesh를 vector에 추가
        //vertex로딩
        loadPBRTTriangleMeshVertices(mesh->vertex, mesh->texcoord, mesh->normal);
        //index로딩
        loadPBRTTriangleMeshIndices(mesh->index);
    }
    void Scene::loadPBRTTriangleMeshVertices(
        _In_ const std::vector<pbrt::vec3f>& vertexPositions,
        _In_ const std::vector<pbrt::vec2f>& vertexUV,
        _In_ const std::vector<pbrt::vec3f>& vertexNormals
    )
    {
        for (UINT i = 0; i < static_cast<UINT>(vertexPositions.size()); i++)
        {
            const pbrt::vec3f& pbrtPosition = vertexPositions[i];
            //const pbrt::vec2f& pbrtUV = vertexUV[i];//.pbrt파일은 텍스처가 없는경우 uv값이 주어지지 않음
            const pbrt::vec3f& pbrtNormal = vertexNormals[i];
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

    void Scene::loadPBRTTriangleMeshIndices(_In_ const std::vector<pbrt::vec3i>& indices)
    {
        for (UINT i = 0u; i < static_cast<UINT>(indices.size()); i++)
        {
            const pbrt::vec3i& pbrtIndex = indices[i];
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
            cb.lumen[i] = XMFLOAT4(m_lights[i]->GetLumen(),0.0f,0.0f,0.0f);
        }
        memcpy(m_pointLightMappedData, &cb, sizeof(cb));
    }
}