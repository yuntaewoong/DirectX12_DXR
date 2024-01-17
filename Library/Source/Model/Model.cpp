#include "pch.h"
#include "Model/Model.h"

#include "assimp/Importer.hpp"	// C++ importer interface
#include "assimp/scene.h"		// output data structure
#include "assimp/postprocess.h"	// post processing flags

namespace library
{
    std::unique_ptr<Assimp::Importer> Model::sm_pImporter = std::make_unique<Assimp::Importer>();

    Model::Model(
        _In_ const std::filesystem::path& filePath,
        _In_ XMVECTOR location,
        _In_ XMVECTOR rotation,
        _In_ XMVECTOR scale
    )   :
        m_filePath(filePath),
        m_meshes(std::vector<std::shared_ptr<Mesh>>()),
        m_materials(std::vector<std::shared_ptr<Material>>()),
        m_pScene(nullptr),
        m_location(location),
        m_rotation(rotation),
        m_scale(scale)
    {}
    HRESULT Model::Initialize(
        _In_ const ComPtr<ID3D12Device>& pDevice,
        _In_ const ComPtr<ID3D12CommandQueue>& pCommandQueue,
        _In_ CBVSRVUAVDescriptorHeap& cbvSrvUavDescriptorHeap
    )
    {
        HRESULT hr = S_OK;
        m_pScene = sm_pImporter->ReadFile(//ASSIMP로 모델 읽기
            m_filePath.string().c_str(),
            aiProcess_Triangulate | aiProcess_GenSmoothNormals |
            aiProcess_CalcTangentSpace | aiProcess_ConvertToLeftHanded
        );
        
        if (m_pScene)
        {
            hr = initFromScene(pDevice,pCommandQueue,cbvSrvUavDescriptorHeap, m_pScene, m_filePath);
        }
        else
        {
            hr = E_FAIL;
            OutputDebugString(L"Error parsing ");
            OutputDebugString(m_filePath.c_str());
            OutputDebugString(L": ");
            OutputDebugStringA(sm_pImporter->GetErrorString());
            OutputDebugString(L"\n");
        }
        if (FAILED(hr))
        {
            return hr;
        }
        return hr;
    }
    void Model::Update(_In_ FLOAT deltaTime)
    {
    }
    void Model::ForceMaterial(_In_ const std::shared_ptr<Material>& material)
    {//외부 material로 모델의 모든 Mesh의 material을 강제로 세팅
        m_materials.push_back(material);
        for (UINT i = 0; i < static_cast<UINT>(m_meshes.size()); i++)
        {
            m_meshes[i]->SetMaterial(m_materials[static_cast<UINT>(m_meshes.size() - 1)]);
        }
    }
    const std::vector<std::shared_ptr<Mesh>>& Model::GetMeshes() const
    {
        return m_meshes;
    }
    const std::vector<std::shared_ptr<Material>>& Model::GetMaterials() const
    {
        return m_materials;
    }
    void Model::setMeshes2Materials(_In_ const aiScene* pScene)
    {
        for (UINT i = 0u; i < pScene->mNumMeshes; i++)
        {
            m_meshes[i]->SetMaterial(m_materials[pScene->mMeshes[i]->mMaterialIndex]);
        }
    }
    HRESULT Model::initAllMeshes(
        _In_ const ComPtr<ID3D12Device>& pDevice,
        _In_ const aiScene* pScene
    )
    {
        HRESULT hr = S_OK;
        for (UINT i = 0u; i < pScene->mNumMeshes; ++i)
        {
            const aiMesh* pMesh = pScene->mMeshes[i];
            hr = initSingleMesh(pDevice,i, pMesh);
            if (FAILED(hr))
            {
                return hr;
            }
        }
        return hr;
    }
    HRESULT Model::initFromScene(
        _In_ const ComPtr<ID3D12Device>& pDevice,
        _In_ const ComPtr<ID3D12CommandQueue>& pCommandQueue,
        _In_ CBVSRVUAVDescriptorHeap& cbvSrvUavDescriptorHeap,
        _In_ const aiScene* pScene,
        _In_ const std::filesystem::path& filePath
    )
    {
        HRESULT hr = S_OK;
        hr = initAllMeshes(pDevice,pScene);
        if (FAILED(hr))
        {
            return hr;
        }
        hr = initMaterials(pDevice, pCommandQueue,cbvSrvUavDescriptorHeap,pScene, filePath);
        if (FAILED(hr))
        {
            return hr;
        }
        setMeshes2Materials(pScene);//모든 modelmesh들을 맞는 material에 매핑
        
        return hr;
    }
    HRESULT Model::initMaterials(
        _In_ const ComPtr<ID3D12Device>& pDevice,
        _In_ const ComPtr<ID3D12CommandQueue>& pCommandQueue,
        _In_ CBVSRVUAVDescriptorHeap& cbvSrvUavDescriptorHeap,
        _In_ const aiScene* pScene,
        _In_ const std::filesystem::path& filePath
    )
    {
        HRESULT hr = S_OK;

        // Extract the directory part from the file name
        std::filesystem::path parentDirectory = filePath.parent_path();

        // Initialize the materials
        for (UINT i = 0u; i < pScene->mNumMaterials; ++i)
        {
            const aiMaterial* pMaterial = pScene->mMaterials[i];
            aiColor3D diffuseColor(0.f, 0.f, 0.f);
            pMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor);//diffuse값 load
            std::shared_ptr<Material> mat = std::make_shared<Material>(XMFLOAT4(diffuseColor.r, diffuseColor.g, diffuseColor.b, 1.f));
            
            aiColor3D emissiveColor(0.f, 0.f, 0.f);
            pMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, emissiveColor);//emissive값 load
            mat->SetEmission(emissiveColor.r*20.f);

            aiColor3D metallicColor(0.f, 0.f, 0.f);
            pMaterial->Get(AI_MATKEY_COLOR_SPECULAR, metallicColor);//metallic값 load
            mat->SetMetallic(metallicColor.r);

            
            m_materials.push_back(mat);
            
            
            loadTextures(pDevice, pCommandQueue,cbvSrvUavDescriptorHeap, parentDirectory, pMaterial, i);
        }

        return hr;
    }
    HRESULT Model::initSingleMesh(_In_ const ComPtr<ID3D12Device>& pDevice,_In_ UINT uMeshIndex, _In_ const aiMesh* pMesh)
    {
        m_meshes.push_back(
            std::make_shared<Mesh>(m_location,m_rotation,m_scale)
        );
        const aiVector3D zero3d(0.0f, 0.0f, 0.0f);
        for (UINT i = 0u; i < pMesh->mNumVertices; i++)
        {
            const aiVector3D& position = pMesh->mVertices[i];
            const aiVector3D& normal = pMesh->mNormals[i];
            const aiVector3D& texCoord = pMesh->HasTextureCoords(0u) ? pMesh->mTextureCoords[0][i] : zero3d;
            const aiVector3D& tangent = pMesh->HasTangentsAndBitangents() ? pMesh->mTangents[i] : zero3d;
            const aiVector3D& biTangent = pMesh->HasTangentsAndBitangents() ? pMesh->mBitangents[i] : zero3d;
            Vertex vertex =
            {
                .position = XMFLOAT3(position.x, position.y, position.z),
                .uv = XMFLOAT2(texCoord.x, texCoord.y),
                .normal = XMFLOAT3(normal.x, normal.y, normal.z),
                .tangent = XMFLOAT3(tangent.x,tangent.y,tangent.z),
                .biTangent = XMFLOAT3(biTangent.x,biTangent.y,biTangent.z)
            };
            m_meshes[uMeshIndex]->AddVertex(vertex);
        }
        for (UINT i = 0u; i < pMesh->mNumFaces; i++)
        {
            const aiFace& face = pMesh->mFaces[i];
            assert(face.mNumIndices == 3u);
            Index aIndices[3] =
            {
                static_cast<Index>(face.mIndices[0]),
                static_cast<Index>(face.mIndices[1]),
                static_cast<Index>(face.mIndices[2]),
            };
            m_meshes[uMeshIndex]->AddIndex(aIndices[0]);
            m_meshes[uMeshIndex]->AddIndex(aIndices[1]);
            m_meshes[uMeshIndex]->AddIndex(aIndices[2]);
        }
        HRESULT hr = m_meshes[uMeshIndex]->Initialize(pDevice);
        if (FAILED(hr))
        {
            return hr;
        }
        return hr;
    }

    HRESULT Model::loadDiffuseTexture(
        _In_ const ComPtr<ID3D12Device>& pDevice,
        _In_ const ComPtr<ID3D12CommandQueue>& pCommandQueue,
        _In_ CBVSRVUAVDescriptorHeap& cbvSrvUavDescriptorHeap,
        _In_ const std::filesystem::path& parentDirectory,
        _In_ const aiMaterial* pMaterial,
        _In_ UINT uIndex
    )
    {
        HRESULT hr = S_OK;
        m_materials[uIndex]->SetAlbedoTexture(nullptr);

        if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0)
        {
            aiString aiPath;

            if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0u, &aiPath, nullptr, nullptr, nullptr, nullptr, nullptr) == AI_SUCCESS)
            {
                std::string szPath(aiPath.data);

                if (szPath.substr(0ull, 2ull) == ".\\")
                {
                    szPath = szPath.substr(2ull, szPath.size() - 2ull);
                }

                std::filesystem::path fullPath = parentDirectory / szPath;

                m_materials[uIndex]->SetAlbedoTexture(std::make_shared<Texture>(fullPath));

                hr = m_materials[uIndex]->GetAlbedoTexture()->Initialize(pDevice,pCommandQueue,cbvSrvUavDescriptorHeap);
                if (FAILED(hr))
                {
                    OutputDebugString(L"Error loading diffuse texture \"");
                    OutputDebugString(fullPath.c_str());
                    OutputDebugString(L"\"\n");

                    return hr;
                }

                OutputDebugString(L"Loaded diffuse texture \"");
                OutputDebugString(fullPath.c_str());
                OutputDebugString(L"\"\n");
            }
        }

        return hr;
    }
    HRESULT Model::loadSpecularTexture(
        _In_ const ComPtr<ID3D12Device>& pDevice,
        _In_ const ComPtr<ID3D12CommandQueue>& pCommandQueue,
        _In_ CBVSRVUAVDescriptorHeap& cbvSrvUavDescriptorHeap,
        _In_ const std::filesystem::path& parentDirectory,
        _In_ const aiMaterial* pMaterial,
        _In_ UINT uIndex
    )
    {
        HRESULT hr = S_OK;
        m_materials[uIndex]->SetSpecularTexture(nullptr);

        if (pMaterial->GetTextureCount(aiTextureType_SPECULAR) > 0)
        {
            aiString aiPath;

            if (pMaterial->GetTexture(aiTextureType_SPECULAR, 0u, &aiPath, nullptr, nullptr, nullptr, nullptr, nullptr) == AI_SUCCESS)
            {
                std::string szPath(aiPath.data);

                if (szPath.substr(0ull, 2ull) == ".\\")
                {
                    szPath = szPath.substr(2ull, szPath.size() - 2ull);
                }

                std::filesystem::path fullPath = parentDirectory / szPath;

                m_materials[uIndex]->SetSpecularTexture(std::make_shared<Texture>(fullPath));

                hr = m_materials[uIndex]->GetSpecularTexture()->Initialize(pDevice,pCommandQueue, cbvSrvUavDescriptorHeap);
                if (FAILED(hr))
                {
                    OutputDebugString(L"Error loading specular texture \"");
                    OutputDebugString(fullPath.c_str());
                    OutputDebugString(L"\"\n");

                    return hr;
                }

                OutputDebugString(L"Loaded specular texture \"");
                OutputDebugString(fullPath.c_str());
                OutputDebugString(L"\"\n");
            }
        }

        return hr;
    }
    HRESULT Model::loadNormalTexture(
        _In_ const ComPtr<ID3D12Device>& pDevice,
        _In_ const ComPtr<ID3D12CommandQueue>& pCommandQueue,
        _In_ CBVSRVUAVDescriptorHeap& cbvSrvUavDescriptorHeap,
        _In_ const std::filesystem::path& parentDirectory,
        _In_ const aiMaterial* pMaterial, _In_ UINT uIndex
    )
    {
        HRESULT hr = S_OK;
        m_materials[uIndex]->SetNormalTexture(nullptr);

        if (pMaterial->GetTextureCount(aiTextureType_HEIGHT) > 0)
        {
            aiString aiPath;

            if (pMaterial->GetTexture(aiTextureType_HEIGHT, 0u, &aiPath, nullptr, nullptr, nullptr, nullptr, nullptr) == AI_SUCCESS)
            {
                std::string szPath(aiPath.data);

                if (szPath.substr(0ull, 2ull) == ".\\")
                {
                    szPath = szPath.substr(2ull, szPath.size() - 2ull);
                }

                std::filesystem::path fullPath = parentDirectory / szPath;

                m_materials[uIndex]->SetNormalTexture(std::make_shared<Texture>(fullPath));
                hr = m_materials[uIndex]->GetNormalTexture()->Initialize(pDevice, pCommandQueue, cbvSrvUavDescriptorHeap);
                //m_bHasNormalMap = true;

                if (FAILED(hr))
                {
                    OutputDebugString(L"Error loading normal texture \"");
                    OutputDebugString(fullPath.c_str());
                    OutputDebugString(L"\"\n");

                    return hr;
                }

                OutputDebugString(L"Loaded normal texture \"");
                OutputDebugString(fullPath.c_str());
                OutputDebugString(L"\"\n");
            }
        }

        return hr;
    }
    HRESULT Model::loadTextures(
        _In_ const ComPtr<ID3D12Device>& pDevice,
        _In_ const ComPtr<ID3D12CommandQueue>& pCommandQueue,
        _In_ CBVSRVUAVDescriptorHeap& cbvSrvUavDescriptorHeap,
        _In_ const std::filesystem::path& parentDirectory,
        _In_ const aiMaterial* pMaterial,
        _In_ UINT uIndex
    )
    {
        HRESULT hr = loadDiffuseTexture(pDevice, pCommandQueue,cbvSrvUavDescriptorHeap, parentDirectory, pMaterial, uIndex);
        if (FAILED(hr))
        {
            return hr;
        }
        hr = loadSpecularTexture(pDevice, pCommandQueue, cbvSrvUavDescriptorHeap,parentDirectory, pMaterial, uIndex);
        if (FAILED(hr))
        {
            return hr;
        }
        hr = loadNormalTexture(pDevice, pCommandQueue, cbvSrvUavDescriptorHeap, parentDirectory, pMaterial, uIndex);
        if (FAILED(hr))
        {
            return hr;
        }
        return hr;
    }
}