
#include "pch.h"
#include "Scene\Scene.h"



namespace library
{

    Scene::Scene(
        _In_ XMVECTOR location,
        _In_ XMVECTOR rotation,
        _In_ XMVECTOR scale
    ) 
        :
        m_filePath(),
        m_meshes(),
        m_pointLights(),
        m_materials(),
        m_bInitialized(FALSE),
        m_location(location),
        m_rotation(rotation),
        m_scale(scale)
    {}

    Scene::Scene(
        _In_ const std::filesystem::path & filePath,
        _In_ XMVECTOR location,
        _In_ XMVECTOR rotation,
        _In_ XMVECTOR scale
    )
        :
        m_filePath(filePath),
        m_meshes(),
        m_pointLights(),
        m_materials(),
        m_bInitialized(FALSE),
        m_location(location),
        m_rotation(rotation),
        m_scale(scale)
    {}

    HRESULT Scene::Initialize(
        _In_ const ComPtr<ID3D12Device>& pDevice,
        _In_ const ComPtr<ID3D12CommandQueue>& pCommandQueue,
        _In_ CBVSRVUAVDescriptorHeap& cbvSrvUavDescriptorHeap
    )
    {
        
        
        if (m_bInitialized)//�ѹ� �ε��Ǿ��� Scene�� �ٽ� �ε��Ҷ� Texture�� �ٽ� �ε�(��ũ���� ���� �ٲ���� ������)
        {
            HRESULT hr = S_OK;
            for (UINT i = 0; i < static_cast<UINT>(m_materials.size()); i++)
            {
                hr = m_materials[i]->Initialize(pDevice, pCommandQueue, cbvSrvUavDescriptorHeap);
                if (FAILED(hr))
                {
                    return hr;
                }
            }
            return hr;
        }
        HRESULT hr = S_OK;
        if (!m_filePath.empty())
        {//���ϰ�ΰ� �־����ٸ�, pbrt �� �ε�
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
        for (UINT i = 0; i < m_materials.size(); i++)
        {
            hr = m_materials[i]->Initialize(pDevice, pCommandQueue, cbvSrvUavDescriptorHeap);
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
            {//�𵨵��� �����ϴ� Mesh�� ���� �߰�
                m_meshes.push_back(meshes[i]);
            }

            const std::vector<std::shared_ptr<Material>>& materials = m_models[i]->GetMaterials();
            for (UINT i = 0u; i < materials.size(); i++)
            {//�𵨵��� �����ϴ� Material�� ���� �߰�
                m_materials.push_back(materials[i]);
            }
        }
        
        m_bInitialized = TRUE;
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
    const std::vector<std::shared_ptr<PointLight>>& Scene::GetPointLights() const
    {
        return m_pointLights;
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
        
    }
    void Scene::loadPBRTWorld(_In_ const std::shared_ptr<const pbrt::Object> world)
    {
        
        for (auto shape : world->shapes) 
        {//shape: pbrt���� primitive����
            if (std::shared_ptr<pbrt::TriangleMesh> mesh = std::dynamic_pointer_cast<pbrt::TriangleMesh>(shape))
            {//TriangleMesh�� �ٿ�ĳ���� ������ Mesh�ε�
                loadPBRTTriangleMesh(mesh);
                if (std::shared_ptr<pbrt::DiffuseAreaLightBB> areaLight = std::dynamic_pointer_cast<pbrt::DiffuseAreaLightBB>(shape->areaLight))
                {//AreaLight�� �ٿ�ĳ���ü����� emissive material ����
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
                {//���н� �Ϲ� material����
                    loadPBRTMaterial(mesh->material);
                }
                m_meshes[m_meshes.size() - 1]->SetMaterial(m_materials[m_materials.size() - 1]);//�ε��� mesh-material ����
            }
        }
        for (auto inst : world->instances) 
        {//Instance�鿡 ���Ͽ� ���ȣ��
            loadPBRTWorld(inst->object);
        }
    }
    void Scene::loadPBRTMaterial(_In_ const std::shared_ptr<pbrt::Material> material)
    {
        std::shared_ptr<Material> mat = std::make_shared<Material>(XMFLOAT4(1.f, 1.f, 1.f, 0.f));
        if (std::shared_ptr<pbrt::MatteMaterial> matMaterial = std::dynamic_pointer_cast<pbrt::MatteMaterial>(material))
        {//MattMaterial�϶�
            mat->SetAlbedo(XMFLOAT4(matMaterial->kd.x,matMaterial->kd.y,matMaterial->kd.z,1.f));
            if (std::shared_ptr<pbrt::ImageTexture> imageAlbedoTexture = std::dynamic_pointer_cast<pbrt::ImageTexture>(matMaterial->map_kd))
            {
                mat->SetAlbedoTexture(std::make_shared<Texture>(m_filePath.parent_path() / imageAlbedoTexture->fileName));
            }
            mat->SetRoughness(matMaterial->sigma / 100.f);

        }
        else if (std::shared_ptr<pbrt::MirrorMaterial> mirrorMaterial = std::dynamic_pointer_cast<pbrt::MirrorMaterial>(material))
        {//MirrorMaterial�϶�
            mat->SetMetallic(mirrorMaterial->kr.x);
        }
        else if (std::shared_ptr<pbrt::PlasticMaterial> placsticMaterial = std::dynamic_pointer_cast<pbrt::PlasticMaterial>(material))
        {//plasticMaterial�϶�
            mat->SetAlbedo(XMFLOAT4(placsticMaterial->kd.x, placsticMaterial->kd.y, placsticMaterial->kd.z, 1.f));
            mat->SetRoughness(placsticMaterial->roughness);
        }
        else if (std::shared_ptr<pbrt::SubstrateMaterial> substrateMaterial = std::dynamic_pointer_cast<pbrt::SubstrateMaterial>(material))
        {//SubstrateMaterial�϶�
            mat->SetAlbedo(XMFLOAT4(substrateMaterial->kd.x, substrateMaterial->kd.y, substrateMaterial->kd.z, 1.f));
            if (std::shared_ptr<pbrt::ImageTexture> imageAlbedoTexture = std::dynamic_pointer_cast<pbrt::ImageTexture>(substrateMaterial->map_kd))
            {
                mat->SetAlbedoTexture(std::make_shared<Texture>(m_filePath.parent_path() / imageAlbedoTexture->fileName));
            }
        }
        else if (std::shared_ptr<pbrt::GlassMaterial> glassMaterial = std::dynamic_pointer_cast<pbrt::GlassMaterial>(material))
        {//GlassMaterial�϶�
            mat->SetMetallic(glassMaterial->kr.x);
        }
        else if (std::shared_ptr<pbrt::UberMaterial> uberMaterial = std::dynamic_pointer_cast<pbrt::UberMaterial>(material))
        {//UberMaterial�϶�
            mat->SetAlbedo(XMFLOAT4(uberMaterial->kd.x, uberMaterial->kd.y, uberMaterial->kd.z, 1.f));
            if (std::shared_ptr<pbrt::ImageTexture> imageAlbedoTexture = std::dynamic_pointer_cast<pbrt::ImageTexture>(uberMaterial->map_kd))
            {
                mat->SetAlbedoTexture(std::make_shared<Texture>(m_filePath.parent_path() / imageAlbedoTexture->fileName));
            }
        }
        else if (std::shared_ptr<pbrt::FourierMaterial> fourierMaterial = std::dynamic_pointer_cast<pbrt::FourierMaterial>(material))
        {//FourierMaterial�϶�
            mat->SetAlbedo(XMFLOAT4(1.f, 1.f, 1.f, 1.f));
            mat->SetRoughness(1.f);
        }
        else if (std::shared_ptr<pbrt::GlassMaterial> glassMaterial = std::dynamic_pointer_cast<pbrt::GlassMaterial>(material))
        {//GlassMaterial�϶�
            mat->SetMetallic(glassMaterial->kr.x);
        }
        else if (std::shared_ptr<pbrt::TranslucentMaterial> translucentMaterialMaterial = std::dynamic_pointer_cast<pbrt::TranslucentMaterial>(material))
        {//TranslucentMaterial�϶�
            //mat->SetAlbedo(XMFLOAT4(0.f, 0.f, 1.f, 1.f));
        }
        else if (std::shared_ptr<pbrt::MetalMaterial> metalMaterial = std::dynamic_pointer_cast<pbrt::MetalMaterial>(material))
        {//MetalMaterial�϶�
            mat->SetRoughness(metalMaterial->roughness);
            mat->SetMetallic(1.f);
        }
        else if (std::shared_ptr<pbrt::MixMaterial> mixMaterial = std::dynamic_pointer_cast<pbrt::MixMaterial>(material))
        {//MixMaterial�϶�
            //mat->SetAlbedo(XMFLOAT4(1.f, 0.f, 0.f, 0.f));
        }
        
        m_materials.push_back(mat);
    }
    void Scene::loadPBRTTriangleMesh(_In_ const std::shared_ptr<const pbrt::TriangleMesh> mesh)
    {
        m_meshes.push_back(
            std::make_shared<Mesh>(
                m_location,
                m_rotation,
                m_scale
            )
        );//�ε��� �� mesh�� vector�� �߰�
        //vertex�ε�
        loadPBRTTriangleMeshVertices(mesh->vertex, mesh->texcoord, mesh->normal);
        //index�ε�
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
            const pbrt::vec2f& pbrtUV = vertexUV.empty() ? pbrt::vec2f(0.f,0.f) : vertexUV[i] ;//.pbrt������ �ؽ�ó�� ���°�� uv���� �־����� ����
            const pbrt::vec3f& pbrtNormal = vertexNormals[i];
            Vertex vertex =
            {
                .position = XMFLOAT3(pbrtPosition.x, pbrtPosition.y, pbrtPosition.z),
                .uv = XMFLOAT2(pbrtUV.x, pbrtUV.y),
                .normal = XMFLOAT3(pbrtNormal.x, pbrtNormal.y, pbrtNormal.z),
                .tangent = XMFLOAT3(0.f,0.f,0.f),//
                .biTangent = XMFLOAT3(0.f,0.f,0.f)
            };//pbrt���� �븻�ʰ���� ���� tangent,bitangent������ ������� �ʴ�
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
}