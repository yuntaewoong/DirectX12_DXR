
#include "pch.h"
#include "Scene\Scene.h"



namespace library
{

    Scene::Scene() 
        :
        m_filePath(),
        m_meshes(),
        m_pointLights(),
        m_materials(),
        m_pointLightsConstantBuffer(nullptr),
        m_areaLightsConstantBuffer(nullptr),
        m_pointLightMappedData(nullptr),
        m_areaLightMappedData(nullptr)
    {}

    Scene::Scene(const std::filesystem::path & filePath)
        :
        m_filePath(filePath),
        m_meshes(),
        m_pointLights(),
        m_materials(),
        m_pointLightsConstantBuffer(nullptr),
        m_areaLightsConstantBuffer(nullptr),
        m_pointLightMappedData(nullptr),
        m_areaLightMappedData(nullptr)
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
            loadPBRTWorld(pbrtScene->world);
            pbrtScene.reset();
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
        for (UINT i = 0; i < m_pointLights.size(); i++)
        {
            hr = m_pointLights[i]->Initialize(pDevice);
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
        hr = createPointLightConstantBuffer(pDevice);
        if (FAILED(hr))
        {
            return hr;
        }
        hr = createAreaLightConstantBuffer(pDevice);
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
        m_pointLights.push_back(pLight);
    }
    void Scene::Update(_In_ FLOAT deltaTime)
    {
        for (UINT i = 0; i < m_meshes.size(); i++)
        {
            m_meshes[i]->Update(deltaTime);
        }
        for (UINT i = 0; i < m_pointLights.size(); i++)
        {
            m_pointLights[i]->Update(deltaTime);
        }
        updatePointLightConstantBuffer();
    }
    ComPtr<ID3D12Resource>& Scene::GetPointLightsConstantBuffer()
    {
        return m_pointLightsConstantBuffer;
    }
    ComPtr<ID3D12Resource>& Scene::GetAreaLightsConstantBuffer()
    {
        return m_areaLightsConstantBuffer;
    }
    void Scene::loadPBRTWorld(_In_ const std::shared_ptr<const pbrt::Object> world)
    {
        
        for (auto shape : world->shapes) 
        {//shape: pbrt씬의 primitive구조
            if (std::shared_ptr<pbrt::TriangleMesh> mesh = std::dynamic_pointer_cast<pbrt::TriangleMesh>(shape))
            {//TriangleMesh로 다운캐스팅 성공시 Mesh로딩
                loadPBRTTriangleMesh(mesh);
                if (std::shared_ptr<pbrt::DiffuseAreaLightBB> areaLight = std::dynamic_pointer_cast<pbrt::DiffuseAreaLightBB>(shape->areaLight))
                {//AreaLight로 다운캐스팅성공시 emissive material 생성
                    std::shared_ptr<Material> emissiveMaterial = std::make_shared<Material>(
                        XMFLOAT4(
                            areaLight->LinRGB().x,
                            areaLight->LinRGB().y,
                            areaLight->LinRGB().z,
                            1.f
                        )
                    );
                    emissiveMaterial->SetEmission(areaLight->scale);
                    m_materials.push_back(emissiveMaterial);
                }
                else
                {//실패시 일반 material생성
                    loadPBRTMaterial(mesh->material);
                }
                m_meshes[m_meshes.size() - 1]->SetMaterial(m_materials[m_materials.size() - 1]);//로딩된 mesh-material 대응
            }
        }
        for (auto inst : world->instances) 
        {//Instance들에 대하여 재귀호출
            loadPBRTWorld(inst->object);
        }
    }
    void Scene::loadPBRTMaterial(_In_ const std::shared_ptr<pbrt::Material> material)
    {
        std::shared_ptr<Material> mat = std::make_shared<Material>(XMFLOAT4(1.f, 1.f, 1.f, 0.f));
        if (std::shared_ptr<pbrt::MatteMaterial> matMaterial = std::dynamic_pointer_cast<pbrt::MatteMaterial>(material))
        {//MattMaterial일때
            mat->SetAlbedo(XMFLOAT4(matMaterial->kd.x,matMaterial->kd.y,matMaterial->kd.z,1.f));
            if (matMaterial->map_kd)
            {//albedo맵이 존재할때

            }
            //mat->SetRoughness(matMaterial->sigma);

        }
        else if (std::shared_ptr<pbrt::MirrorMaterial> mirrorMaterial = std::dynamic_pointer_cast<pbrt::MirrorMaterial>(material))
        {//MirrorMaterial일때
            mat->SetMetallic(mirrorMaterial->kr.x);
        }
        else if (std::shared_ptr<pbrt::PlasticMaterial> placsticMaterial = std::dynamic_pointer_cast<pbrt::PlasticMaterial>(material))
        {//plasticMaterial일때
            mat->SetAlbedo(XMFLOAT4(placsticMaterial->kd.x, placsticMaterial->kd.y, placsticMaterial->kd.z, 1.f));
            mat->SetRoughness(placsticMaterial->roughness);
        }
        else if (std::shared_ptr<pbrt::SubstrateMaterial> substrateMaterial = std::dynamic_pointer_cast<pbrt::SubstrateMaterial>(material))
        {//SubstrateMaterial일때
            mat->SetAlbedo(XMFLOAT4(substrateMaterial->kd.x, substrateMaterial->kd.y, substrateMaterial->kd.z, 1.f));
            if (std::shared_ptr<pbrt::ImageTexture> imageAlbedoTexture = std::dynamic_pointer_cast<pbrt::ImageTexture>(substrateMaterial->map_kd))
            {
                mat->SetAlbedoTexture(std::make_shared<Texture>(m_filePath.parent_path() / imageAlbedoTexture->fileName));
            }
        }
        else if (std::shared_ptr<pbrt::GlassMaterial> glassMaterial = std::dynamic_pointer_cast<pbrt::GlassMaterial>(material))
        {//GlassMaterial일때
            mat->SetMetallic(glassMaterial->kr.x);
        }
        else if (std::shared_ptr<pbrt::UberMaterial> uberMaterial = std::dynamic_pointer_cast<pbrt::UberMaterial>(material))
        {//UberMaterial일때
            mat->SetAlbedo(XMFLOAT4(uberMaterial->kd.x, uberMaterial->kd.y, uberMaterial->kd.z, 1.f));
            if (std::shared_ptr<pbrt::ImageTexture> imageAlbedoTexture = std::dynamic_pointer_cast<pbrt::ImageTexture>(uberMaterial->map_kd))
            {
                mat->SetAlbedoTexture(std::make_shared<Texture>(m_filePath.parent_path() / imageAlbedoTexture->fileName));
            }
        }
        else if (std::shared_ptr<pbrt::FourierMaterial> fourierMaterial = std::dynamic_pointer_cast<pbrt::FourierMaterial>(material))
        {//FourierMaterial일때
            mat->SetAlbedo(XMFLOAT4(1.f, 1.f, 0.f, 1.f));
            auto a = fourierMaterial->fileName;
            auto b = 1;
        }
        
        

        m_materials.push_back(mat);
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
            const pbrt::vec2f& pbrtUV = vertexUV.empty() ? pbrt::vec2f(0.f,0.f) : vertexUV[i] ;//.pbrt파일은 텍스처가 없는경우 uv값이 주어지지 않음
            const pbrt::vec3f& pbrtNormal = vertexNormals[i];
            Vertex vertex =
            {
                .position = XMFLOAT3(pbrtPosition.x, pbrtPosition.y, pbrtPosition.z),
                .uv = XMFLOAT2(pbrtUV.x, pbrtUV.y),
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
    HRESULT Scene::createPointLightConstantBuffer(_In_ const ComPtr<ID3D12Device>& pDevice)
    {//point light들에 대한 constant buffer만들기
        HRESULT hr = S_OK;
        const D3D12_HEAP_PROPERTIES uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

        size_t cbSize = 256;//Constant버퍼는 256의 배수여야함
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
        updatePointLightConstantBuffer();
        return hr;
    }
    HRESULT Scene::createAreaLightConstantBuffer(_In_ const ComPtr<ID3D12Device>& pDevice)
    {
        HRESULT hr = S_OK;
        const D3D12_HEAP_PROPERTIES uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

        size_t cbSize = 256 * NUM_AREA_LIGHT_MAX;//Constant버퍼는 256의 배수여야함
        const D3D12_RESOURCE_DESC constantBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(cbSize);

        hr = pDevice->CreateCommittedResource(
            &uploadHeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &constantBufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_areaLightsConstantBuffer)
        );
        if (FAILED(hr))
        {
            return hr;
        }
        CD3DX12_RANGE readRange(0, 0);
        hr = m_areaLightsConstantBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_areaLightMappedData));
        if (FAILED(hr))
        {
            return hr;
        }
        updateAreaLightConstantBuffer();
        return hr;
    }
    void Scene::updatePointLightConstantBuffer()
    {
        PointLightConstantBuffer cb = {};
        for (UINT i = 0; i < m_pointLights.size(); i++)
        {
            cb.position[i] = m_pointLights[i]->GetPosition();
            cb.lumen[i] = XMFLOAT4(m_pointLights[i]->GetLumen(),0.0f,0.0f,0.0f);
        }
        cb.numPointLight = static_cast<UINT>(m_pointLights.size());
        memcpy(m_pointLightMappedData, &cb, sizeof(cb));
    }
    void Scene::updateAreaLightConstantBuffer()
    {
        //Area Light정의: Emission값이 0을 초과하는 Mesh가 발산하는 빛
        AreaLightConstantBuffer cb = {};
        UINT numAreaLights = 0u;
        for (UINT i = 0; i < static_cast<UINT>(m_meshes.size()); i++)
        {
            if (m_meshes[i]->GetMaterial()->GetEmission() > 0.001f)
            {
                
                for (UINT j = 0; j < static_cast<UINT>(m_meshes[i]->GetIndices().size()); j += 3)
                {//polygon loop
                    cb.worldMatrix[numAreaLights] = m_meshes[i]->GetWorldMatrix();
                    cb.normal[numAreaLights] = XMVectorSet(
                        m_meshes[i]->GetVertices()[m_meshes[i]->GetIndices()[j].index].normal.x,
                        m_meshes[i]->GetVertices()[m_meshes[i]->GetIndices()[j].index].normal.y,
                        m_meshes[i]->GetVertices()[m_meshes[i]->GetIndices()[j].index].normal.z,
                        1.f
                    );
                    for (UINT k = 0; k < 3u; k++)
                    {
                        cb.vertices[numAreaLights * 3 + k] = XMVectorSet(
                            m_meshes[i]->GetVertices()[m_meshes[i]->GetIndices()[j+k].index].position.x,
                            m_meshes[i]->GetVertices()[m_meshes[i]->GetIndices()[j+k].index].position.y,
                            m_meshes[i]->GetVertices()[m_meshes[i]->GetIndices()[j+k].index].position.z,
                            1.f
                        );
                    }
                    cb.lightColor[numAreaLights] = m_meshes[i]->GetMaterial()->GetAlbedo();
                    cb.emission[numAreaLights] = XMFLOAT4(m_meshes[i]->GetMaterial()->GetEmission(),0.f,0.f,0.f);
                    numAreaLights++;
                }
            }
        }
        
        cb.numAreaLight = numAreaLights;
        memcpy(m_areaLightMappedData, &cb, sizeof(cb));
    }
}