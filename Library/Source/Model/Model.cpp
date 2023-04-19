#include "Model\Model.h"
#include "pch.h"
//#include "Model/Model.h"
//
//#include "assimp/Importer.hpp"	// C++ importer interface
//#include "assimp/scene.h"		// output data structure
//#include "assimp/postprocess.h"	// post processing flags
//
//namespace library
//{
//    XMMATRIX ConvertMatrix(_In_ const aiMatrix4x4& matrix)
//    {
//        return XMMATRIX(
//            matrix.a1,
//            matrix.b1,
//            matrix.c1,
//            matrix.d1,
//            matrix.a2,
//            matrix.b2,
//            matrix.c2,
//            matrix.d2,
//            matrix.a3,
//            matrix.b3,
//            matrix.c3,
//            matrix.d3,
//            matrix.a4,
//            matrix.b4,
//            matrix.c4,
//            matrix.d4
//        );
//    }
//    XMFLOAT3 ConvertVector3dToFloat3(_In_ const aiVector3D& vector)
//    {
//        return XMFLOAT3(vector.x, vector.y, vector.z);
//    }
//    XMVECTOR ConvertQuaternionToVector(_In_ const aiQuaternion& quaternion)
//    {
//        XMFLOAT4 float4 = XMFLOAT4(quaternion.x, quaternion.y, quaternion.z, quaternion.w);
//        return XMLoadFloat4(&float4);
//    }
//    std::unique_ptr<Assimp::Importer> Model::sm_pImporter = std::make_unique<Assimp::Importer>();
//
//    Model::Model(
//        _In_ const std::filesystem::path& filePath,
//        _In_ XMVECTOR location,
//        _In_ XMVECTOR rotation,
//        _In_ XMVECTOR scale
//    )   :
//        m_filePath(filePath),
//        m_aMeshes(std::vector<std::shared_ptr<Renderable>>()),
//        m_aTransforms(std::vector<XMMATRIX>()),
//        m_pScene(nullptr),
//        m_timeSinceLoaded(0.0f)
//    {}
//    HRESULT Model::Initialize(_In_ const ComPtr<ID3D12Device>& pDevice)
//    {
//        HRESULT hr = S_OK;
//        m_pScene = sm_pImporter->ReadFile(//ASSIMP·Î ¸ðµ¨ ÀÐ±â
//            m_filePath.string().c_str(),
//            aiProcess_Triangulate | aiProcess_GenSmoothNormals |
//            aiProcess_CalcTangentSpace | aiProcess_ConvertToLeftHanded
//        );
//        if (m_pScene)
//        {
//            hr = initFromScene(pDevice, m_pScene, m_filePath);
//        }
//        else
//        {
//            hr = E_FAIL;
//            OutputDebugString(L"Error parsing ");
//            OutputDebugString(m_filePath.c_str());
//            OutputDebugString(L": ");
//            OutputDebugStringA(sm_pImporter->GetErrorString());
//            OutputDebugString(L"\n");
//            return hr;
//        }
//        if (FAILED(hr))
//        {
//            return hr;
//        }
//        return hr;
//    }
//    void Model::Update(_In_ FLOAT deltaTime)
//    {
//    }
//    const std::vector<Renderable>& Model::GetMeshes() const
//    {
//        return m_aMeshes;
//    }
//    void Model::countVerticesAndIndices(_Inout_ UINT& uOutNumVertices, _Inout_ UINT& uOutNumIndices, _In_ const aiScene* pScene)
//    {
//        for (UINT i = 0u; i < pScene->mNumMeshes; i++)
//        {
//            m_aMeshes[i].uMaterialIndex = pScene->mMeshes[i]->mMaterialIndex;
//            m_aMeshes[i].uBaseIndex = uOutNumIndices;
//            m_aMeshes[i].uBaseVertex = uOutNumVertices;
//            m_aMeshes[i].uNumIndices = pScene->mMeshes[i]->mNumFaces * 3u;
//
//            uOutNumIndices += pScene->mMeshes[i]->mNumFaces * 3u;
//            uOutNumVertices += pScene->mMeshes[i]->mNumVertices;
//        }
//    }
//    void Model::initAllMeshes(_In_ const aiScene* pScene)
//    {
//        for (UINT i = 0u; i < m_aMeshes.size(); ++i)
//        {
//            const aiMesh* pMesh = pScene->mMeshes[i];
//            initSingleMesh(i, pMesh);
//        }
//    }
//    HRESULT Model::initFromScene(
//        _In_ const ComPtr<ID3D12Device>& pDevice,
//        _In_ const aiScene* pScene,
//        _In_ const std::filesystem::path& filePath
//    )
//    {
//        HRESULT hr = S_OK;
//        m_aMeshes.resize(pScene->mNumMeshes);
//        //UINT uNumVertices = 0u;
//        //UINT uNumIndices = 0u;
//        //countVerticesAndIndices(uNumVertices, uNumIndices, pScene);
//        //reserveSpace(uNumVertices, uNumIndices);
//        initAllMeshes(pScene);
//        hr = initMaterials(pDevice, pScene, filePath);
//        if (FAILED(hr))
//        {
//            return hr;
//        }
//        return hr;
//    }
//    HRESULT Model::initMaterials(
//        _In_ const ComPtr<ID3D12Device>& pDevice,
//        _In_ const aiScene* pScene,
//        _In_ const std::filesystem::path& filePath
//    )
//    {
//        HRESULT hr = S_OK;
//
//        // Extract the directory part from the file name
//        std::filesystem::path parentDirectory = filePath.parent_path();
//
//        // Initialize the materials
//        for (UINT i = 0u; i < pScene->mNumMaterials; ++i)
//        {
//            const aiMaterial* pMaterial = pScene->mMaterials[i];
//
//            std::string szName = filePath.string() + std::to_string(i);
//            std::wstring pwszName(szName.length(), L' ');
//            std::copy(szName.begin(), szName.end(), pwszName.begin());
//            m_aMaterials.push_back(std::make_shared<Material>(pwszName));
//
//            loadTextures(pDevice, pImmediateContext, parentDirectory, pMaterial, i);
//        }
//
//        return hr;
//    }
//    void Model::initSingleMesh(_In_ UINT uMeshIndex, _In_ const aiMesh* pMesh)
//    {
//        const aiVector3D zero3d(0.0f, 0.0f, 0.0f);
//        for (UINT i = 0u; i < pMesh->mNumVertices; i++)
//        {
//            const aiVector3D& position = pMesh->mVertices[i];
//            const aiVector3D& normal = pMesh->mNormals[i];
//            const aiVector3D& texCoord = pMesh->HasTextureCoords(0u) ?
//                pMesh->mTextureCoords[0][i] : zero3d;
//            const aiVector3D& tangent = pMesh->HasTangentsAndBitangents() ?
//                pMesh->mTangents[i] : zero3d;
//            const aiVector3D& bitangent = pMesh->HasTangentsAndBitangents() ?
//                pMesh->mBitangents[i] : zero3d;
//            Vertex vertex =
//            {
//                .Position = XMFLOAT3(position.x, position.y, position.z),
//                .TexCoord = XMFLOAT2(texCoord.x, texCoord.y),
//                .Normal = XMFLOAT3(normal.x, normal.y, normal.z)
//            };
//            NormalData normalData =
//            {
//                .Tangent = XMFLOAT3(tangent.x,tangent.y,tangent.z),
//                .Bitangent = XMFLOAT3(bitangent.x,bitangent.y,bitangent.z)
//            };
//            m_aVertices.push_back(vertex);
//            m_aNormalData.push_back(normalData);
//        }
//        for (UINT i = 0u; i < pMesh->mNumFaces; i++)
//        {
//            const aiFace& face = pMesh->mFaces[i];
//            assert(face.mNumIndices == 3u);
//            WORD aIndices[3] =
//            {
//                static_cast<WORD>(face.mIndices[0]),
//                static_cast<WORD>(face.mIndices[1]),
//                static_cast<WORD>(face.mIndices[2]),
//            };
//            m_aIndices.push_back(aIndices[0]);
//            m_aIndices.push_back(aIndices[1]);
//            m_aIndices.push_back(aIndices[2]);
//        }
//    }
//
//    HRESULT Model::loadDiffuseTexture(
//        _In_ const ComPtr<ID3D12Device>& pDevice,
//        _In_ const std::filesystem::path& parentDirectory,
//        _In_ const aiMaterial* pMaterial,
//        _In_ UINT uIndex
//    )
//    {
//        HRESULT hr = S_OK;
//        m_aMaterials[uIndex]->pDiffuse = nullptr;
//
//        if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0)
//        {
//            aiString aiPath;
//
//            if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0u, &aiPath, nullptr, nullptr, nullptr, nullptr, nullptr) == AI_SUCCESS)
//            {
//                std::string szPath(aiPath.data);
//
//                if (szPath.substr(0ull, 2ull) == ".\\")
//                {
//                    szPath = szPath.substr(2ull, szPath.size() - 2ull);
//                }
//
//                std::filesystem::path fullPath = parentDirectory / szPath;
//
//                m_aMaterials[uIndex]->pDiffuse = std::make_shared<Texture>(fullPath);
//
//                hr = m_aMaterials[uIndex]->pDiffuse->Initialize(pDevice, pImmediateContext);
//                if (FAILED(hr))
//                {
//                    OutputDebugString(L"Error loading diffuse texture \"");
//                    OutputDebugString(fullPath.c_str());
//                    OutputDebugString(L"\"\n");
//
//                    return hr;
//                }
//
//                OutputDebugString(L"Loaded diffuse texture \"");
//                OutputDebugString(fullPath.c_str());
//                OutputDebugString(L"\"\n");
//            }
//        }
//
//        return hr;
//    }
//    HRESULT Model::loadSpecularTexture(
//        _In_ const ComPtr<ID3D12Device>& pDevice,
//        _In_ const std::filesystem::path& parentDirectory,
//        _In_ const aiMaterial* pMaterial,
//        _In_ UINT uIndex
//    )
//    {
//        HRESULT hr = S_OK;
//        m_aMaterials[uIndex]->pSpecularExponent = nullptr;
//
//        if (pMaterial->GetTextureCount(aiTextureType_SHININESS) > 0)
//        {
//            aiString aiPath;
//
//            if (pMaterial->GetTexture(aiTextureType_SHININESS, 0u, &aiPath, nullptr, nullptr, nullptr, nullptr, nullptr) == AI_SUCCESS)
//            {
//                std::string szPath(aiPath.data);
//
//                if (szPath.substr(0ull, 2ull) == ".\\")
//                {
//                    szPath = szPath.substr(2ull, szPath.size() - 2ull);
//                }
//
//                std::filesystem::path fullPath = parentDirectory / szPath;
//
//                m_aMaterials[uIndex]->pSpecularExponent = std::make_shared<Texture>(fullPath);
//
//                hr = m_aMaterials[uIndex]->pSpecularExponent->Initialize(pDevice, pImmediateContext);
//                if (FAILED(hr))
//                {
//                    OutputDebugString(L"Error loading specular texture \"");
//                    OutputDebugString(fullPath.c_str());
//                    OutputDebugString(L"\"\n");
//
//                    return hr;
//                }
//
//                OutputDebugString(L"Loaded specular texture \"");
//                OutputDebugString(fullPath.c_str());
//                OutputDebugString(L"\"\n");
//            }
//        }
//
//        return hr;
//    }
//    HRESULT Model::loadNormalTexture(
//        _In_ const ComPtr<ID3D12Device>& pDevice,
//        _In_ const std::filesystem::path& parentDirectory,
//        _In_ const aiMaterial* pMaterial, _In_ UINT uIndex
//    )
//    {
//        HRESULT hr = S_OK;
//        m_aMaterials[uIndex]->pNormal = nullptr;
//
//        if (pMaterial->GetTextureCount(aiTextureType_HEIGHT) > 0)
//        {
//            aiString aiPath;
//
//            if (pMaterial->GetTexture(aiTextureType_HEIGHT, 0u, &aiPath, nullptr, nullptr, nullptr, nullptr, nullptr) == AI_SUCCESS)
//            {
//                std::string szPath(aiPath.data);
//
//                if (szPath.substr(0ull, 2ull) == ".\\")
//                {
//                    szPath = szPath.substr(2ull, szPath.size() - 2ull);
//                }
//
//                std::filesystem::path fullPath = parentDirectory / szPath;
//
//                m_aMaterials[uIndex]->pNormal = std::make_shared<Texture>(fullPath);
//                m_bHasNormalMap = true;
//
//                if (FAILED(hr))
//                {
//                    OutputDebugString(L"Error loading normal texture \"");
//                    OutputDebugString(fullPath.c_str());
//                    OutputDebugString(L"\"\n");
//
//                    return hr;
//                }
//
//                OutputDebugString(L"Loaded normal texture \"");
//                OutputDebugString(fullPath.c_str());
//                OutputDebugString(L"\"\n");
//            }
//        }
//
//        return hr;
//    }
//    HRESULT Model::loadTextures(
//        _In_ const ComPtr<ID3D12Device>& pDevice,
//        _In_ const std::filesystem::path& parentDirectory,
//        _In_ const aiMaterial* pMaterial,
//        _In_ UINT uIndex
//    )
//    {
//        HRESULT hr = loadDiffuseTexture(pDevice, parentDirectory, pMaterial, uIndex);
//        if (FAILED(hr))
//        {
//            return hr;
//        }
//        hr = loadSpecularTexture(pDevice,  parentDirectory, pMaterial, uIndex);
//        if (FAILED(hr))
//        {
//            return hr;
//        }
//        hr = loadNormalTexture(pDevice, parentDirectory, pMaterial, uIndex);
//        if (FAILED(hr))
//        {
//            return hr;
//        }
//        return hr;
//    }
//    void Model::reserveSpace(_In_ UINT uNumVertices, _In_ UINT uNumIndices)
//    {
//        m_aVertices.reserve(uNumVertices);
//        m_aIndices.reserve(uNumIndices);
//        m_aBoneData.resize(uNumVertices);
//    }
//}