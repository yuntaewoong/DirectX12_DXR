#pragma once
#include "Common/Common.h"
#include "Render\Renderable.h"

struct aiScene;
struct aiMesh;
struct aiMaterial;
struct aiAnimation;
struct aiBone;
struct aiNode;
struct aiNodeAnim;

namespace Assimp
{
    class Importer;
}

namespace library
{
    class Model
    {
    public:
        Model() = delete;
        Model(
            _In_ const std::filesystem::path& filePath,
            _In_ XMVECTOR location,
            _In_ XMVECTOR rotation,
            _In_ XMVECTOR scale
        );
        Model(const Model& other) = delete;
        Model(Model&& other) = delete;
        Model& operator=(const Model& other) = delete;
        Model& operator=(Model&& other) = delete;
        ~Model() = default;

        HRESULT Initialize(_In_ const ComPtr<ID3D12Device>& pDevice);
        void Update(_In_ FLOAT deltaTime);
        const std::vector<std::shared_ptr<Renderable>>& GetMeshes() const;
    private:
        void countVerticesAndIndices(_Inout_ UINT& uOutNumVertices, _Inout_ UINT& uOutNumIndices, _In_ const aiScene* pScene);
        void initAllMeshes(_In_ const aiScene* pScene);
        HRESULT initFromScene(
            _In_ const ComPtr<ID3D12Device>& pDevice,
            _In_ const aiScene* pScene,
            _In_ const std::filesystem::path& filePath
        );
        HRESULT initMaterials(
            _In_ const ComPtr<ID3D12Device>& pDevice,
            _In_ const aiScene* pScene,
            _In_ const std::filesystem::path& filePath
        );
        virtual void initSingleMesh(_In_ UINT uMeshIndex, _In_ const aiMesh* pMesh);
        HRESULT loadDiffuseTexture(
            _In_ const ComPtr<ID3D12Device>& pDevice,
            _In_ const std::filesystem::path& parentDirectory,
            _In_ const aiMaterial* pMaterial,
            _In_ UINT uIndex
        );
        HRESULT loadSpecularTexture(
            _In_ const ComPtr<ID3D12Device>& pDevice,
            _In_ const std::filesystem::path& parentDirectory,
            _In_ const aiMaterial* pMaterial,
            _In_ UINT uIndex
        );
        HRESULT loadNormalTexture(
            _In_ const ComPtr<ID3D12Device>& pDevice,
            _In_ const std::filesystem::path& parentDirectory,
            _In_ const aiMaterial* pMaterial,
            _In_ UINT uIndex
        );
        HRESULT loadTextures(
            _In_ const ComPtr<ID3D12Device>& pDevice,
            _In_ const std::filesystem::path& parentDirectory,
            _In_ const aiMaterial* pMaterial,
            _In_ UINT uIndex
        );
        void reserveSpace(_In_ UINT uNumVertices, _In_ UINT uNumIndices);
    private:
        static std::unique_ptr<Assimp::Importer> sm_pImporter;
    private:
        std::filesystem::path m_filePath;
        std::vector<std::shared_ptr<Renderable>> m_aMeshes;
        std::vector<XMMATRIX> m_aTransforms;
        const aiScene* m_pScene;
        float m_timeSinceLoaded;
    };
}